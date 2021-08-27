#pragma once

#include <cstring>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <assert.h>

#include <fstream>
#include <functional>

#include "spinlock.hpp"

namespace obps
{

template <size_t msg_size, size_t polling_micros>
class LogQueue final
{
static_assert(msg_size > 0, "Message size must be greater than 0!");
static_assert(msg_size <= 1024, "Max Message size is 1024 bytes!");

public:
    static const size_t max_message_size = msg_size;

public:
    explicit LogQueue(size_t queue_size);
    ~LogQueue();

    uint16_t Read(char *out);
    uint16_t ReadToFile(std::ostream& dest);
    uint16_t ReadTo(std::function<void(const char *msg, uint16_t size)> f);

    void Write(const char *in, uint16_t size);
    void Write(std::istream& in_stream, uint16_t size);
    
    bool isAlive() const;


    bool isReadAvailable() const;
    bool isWriteAvailable() const;

    size_t GetMessagesCount() const;

    size_t GetSize() const;

    void ShutDownReaders();
    void ShutDownWriters();
    void ShutDown();

private:
    void ReadDecorator(std::function<void(void)> read_impl);
    void WriteDecorator(std::function<void(void)> write_impl);

    struct Message
    {
        uint16_t Size;
        char Buffer[msg_size];
    };

    const size_t m_QueueSize;
    std::vector<Message> m_Messages;

    typename std::vector<Message>::const_iterator m_Reader;
    typename std::vector<Message>::iterator       m_Writer;


    std::mutex m_ReaderMutex;
    std::condition_variable m_ReadCV;
    spinlock writer_spinlock;
  
    std::atomic<size_t> m_num_of_waiting_readers = 0;
    std::atomic<size_t> messages_available_for_reading = 0;

    bool m_Alive = true;
    bool m_ShutDownReaders = false;
    bool m_ShutDownWriters = false;
};


template <size_t msg_size, size_t polling_micros>
bool LogQueue<msg_size, polling_micros>::isAlive() const
{ 
    return m_Alive;
}


template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::ShutDownReaders() 
{ 
    m_ShutDownReaders = true;

    while(m_num_of_waiting_readers.load(std::memory_order_relaxed))
    {
        m_ReadCV.notify_all(); 
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}


template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::ShutDownWriters() 
{ 
    m_ShutDownWriters = true;
}


template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::ShutDown() 
{ 
    ShutDownWriters();
    ShutDownReaders();

    m_Alive = false;
}


template <size_t msg_size, size_t polling_micros>
bool LogQueue<msg_size, polling_micros>::isReadAvailable() const 
{
    return GetMessagesCount() > 0; 
}


template <size_t msg_size, size_t polling_micros>
bool LogQueue<msg_size, polling_micros>::isWriteAvailable() const
{
    return GetMessagesCount() < m_QueueSize;
}


template <size_t msg_size, size_t polling_micros>
size_t LogQueue<msg_size, polling_micros>::GetMessagesCount() const {
    return messages_available_for_reading.load(std::memory_order_relaxed);
}


template <size_t msg_size, size_t polling_micros>
size_t LogQueue<msg_size, polling_micros>::GetSize() const {
    return m_QueueSize;
}


template <size_t msg_size, size_t polling_micros>
LogQueue<msg_size, polling_micros>::LogQueue(size_t queue_size)
  : m_QueueSize(queue_size)
  , m_Messages(queue_size)
  , m_Reader(m_Messages.cbegin())
  , m_Writer(m_Messages.begin())
{
    assert(queue_size > 0 && "Queue size must be greater than 0!");
}


template <size_t msg_size, size_t polling_micros>
LogQueue<msg_size, polling_micros>::~LogQueue()
{
    ShutDown();
    m_ReadCV.notify_all();
}


template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::ReadDecorator(std::function<void(void)> read_impl)
{
    auto critical_section_read = [this, &read_impl]{
        auto lock_ = std::unique_lock<std::mutex>(m_ReaderMutex);

        if (isReadAvailable())
        {
            if (m_Reader == m_Messages.cend())
            {
                m_Reader = m_Messages.cbegin();
            }

            read_impl();
            m_Reader = std::next(m_Reader);

            messages_available_for_reading.fetch_sub(1, std::memory_order_relaxed);
        }
    };

    m_num_of_waiting_readers.fetch_add(1, std::memory_order_relaxed);

    // Wait
    {
        auto lock_ = std::unique_lock<std::mutex>(m_ReaderMutex);

        m_ReadCV.wait(lock_, [this]{ return isReadAvailable() || m_ShutDownReaders; });

        if (m_ShutDownReaders)
        {
            while(isReadAvailable())
            {
                critical_section_read(); // complete remaining reads before shutdown 
            }

            m_num_of_waiting_readers.fetch_sub(1, std::memory_order_relaxed);

            return;
        }
    }

    // Critical
    critical_section_read();

    m_num_of_waiting_readers.fetch_sub(1, std::memory_order_relaxed);
}


template <size_t msg_size, size_t polling_micros>
uint16_t LogQueue<msg_size, polling_micros>::Read(char *out)
{
    uint16_t size;
    
    ReadDecorator([this, &out, &size]{
        size = m_Reader->Size;
        std::memcpy(out, m_Reader->Buffer, size);
    });

    return size;
}


template <size_t msg_size, size_t polling_micros>
uint16_t LogQueue<msg_size, polling_micros>::ReadToFile(std::ostream& dest)
{
    uint16_t size;

    ReadDecorator([this, &dest, &size]{
        size = m_Reader->Size;
        dest.write(m_Reader->Buffer, size); 
    });

    return size;
}


template <size_t msg_size, size_t polling_micros>
uint16_t LogQueue<msg_size, polling_micros>::ReadTo(std::function<void(const char *msg, uint16_t size)> f)
{
    uint16_t size;

    ReadDecorator([this, &f, &size]{
        size = m_Reader->Size;
        f(m_Reader->Buffer, size);
    });

    return size;
}


template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::WriteDecorator(std::function<void(void)> write_impl)
{
    for(;;) {
    // Wait until write available
    while (!isWriteAvailable() || m_ShutDownWriters)
        std::this_thread::sleep_for(std::chrono::microseconds(polling_micros)); // spin
    
    if (m_ShutDownWriters)
    {
        return;
    }

    writer_spinlock.lock();
    if (isWriteAvailable())
    {
        if (m_Writer == m_Messages.end())
        {
            m_Writer = m_Messages.begin();
        }

        write_impl();
        m_Writer = std::next(m_Writer);

        messages_available_for_reading.fetch_add(1, std::memory_order_relaxed);
        
        writer_spinlock.unlock();
        m_ReadCV.notify_one();
        
        break; // job done, leave
    }
    else 
    {
        writer_spinlock.unlock();
        continue; // try again
    }
    }
}


template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::Write(const char *in, uint16_t size)
{
    WriteDecorator([this, &in, &size](){
        std::memcpy(m_Writer->Buffer, in, size);
        m_Writer->Size = size;
    });
}


template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::Write(std::istream& in_stream, uint16_t size)
{
    WriteDecorator([this, &in_stream, &size](){
        in_stream.read(m_Writer->Buffer, size);
        
        m_Writer->Size = size;
    });
}

} // namespace obps