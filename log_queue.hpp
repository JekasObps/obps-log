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

template <size_t msg_size>
class LogQueue final
{
    static_assert(msg_size > 0, "Message size must be greater than 0!");
    static_assert(msg_size <= 1024, "Max Message size is 1024 bytes!");
    static_assert(msg_size % sizeof(size_t) == 0, "message must be aligned to word size");

public:
    enum class OperationStatus {SUCCESS, SHUTDOWN, FAIL};

    typedef uint16_t MessageSize_t;

    static constexpr size_t max_message_size = msg_size;
    static constexpr size_t MessageSize_typesize = sizeof(MessageSize_t);

public:
    explicit LogQueue(size_t queue_size);
    ~LogQueue();

    using UserReadCallback = std::function<void(const char * const queue_message_buffer, uint16_t queue_message_size)>;
    // 
    // queue_message_size -> is a property that user have to set to the size of a data it puts inside a queue 
    using UserWriteCallback = std::function<void(char * const queue_message_buffer, uint16_t & queue_message_size_out)>;
    
    /**
    * Removing From The Queue: */
    OperationStatus ReadTo(char *out);
    OperationStatus ReadTo(std::ostream& dest);
    OperationStatus ReadTo(UserReadCallback callback);
    
    template<typename M>
    OperationStatus ReadEmplace(M * memory);

    template<typename M>
    static void Construct(M * memory, const char* buffer) noexcept;

    /**
    * Inserting Into The Queue: */
    OperationStatus WriteFrom(const char *in, uint16_t size);
    OperationStatus WriteFrom(std::istream& in_stream, uint16_t size);
    OperationStatus WriteFrom(UserWriteCallback callback);
    
    template<typename M, typename ...Args>
    OperationStatus WriteEmplace(Args&& ...args);

    bool isAlive() const noexcept;
    bool isReadAvailable() const noexcept;
    bool isWriteAvailable() const noexcept;
    
    size_t GetMessagesCount() const noexcept;
    size_t GetSize() const noexcept;

    void ShutDownReaders();
    void ShutDownWriters() noexcept;
    void ShutDown();

private:
    OperationStatus ReadDecorator(const std::function<void(void)> read_impl);
    OperationStatus ReadCriticalSection(const std::function<void(void)> &read_impl);

    OperationStatus WriteDecorator(std::function<void(void)> write_impl);

    struct Message
    {
        MessageSize_t Size;
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
}; // class LogQueue


template <size_t msg_size>
LogQueue<msg_size>::LogQueue(size_t queue_size)
  : m_QueueSize(queue_size)
  , m_Messages(queue_size)
  , m_Reader(m_Messages.cbegin())
  , m_Writer(m_Messages.begin())
{
    assert(queue_size > 0 && "Queue size must be greater than 0!");
}

template <size_t msg_size>
LogQueue<msg_size>::~LogQueue()
{
    ShutDown();
    m_ReadCV.notify_all();
}

template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::ReadTo(char *out)
{
    return ReadDecorator([this, &out]{
        std::memcpy(out, m_Reader->Buffer, m_Reader->Size);
    });
}

template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::ReadTo(std::ostream& dest)
{
    return ReadDecorator([this, &dest]{
        dest.write(m_Reader->Buffer, m_Reader->Size); 
    });
}


template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::ReadTo(UserReadCallback callback)
{
    return ReadDecorator([this, &callback]{
        callback(m_Reader->Buffer, m_Reader->Size);
    });
}

template<size_t msg_size>
template<typename M>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::ReadEmplace(M * memory)
{
    return ReadDecorator([this, &memory]{
        Construct(memory, m_Reader->Buffer);
    });
}

template<size_t msg_size>
template<typename M>
void LogQueue<msg_size>::Construct(M * memory, const char* buffer) noexcept
{
    new (memory) M(*reinterpret_cast<const M*>(buffer));
}

template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::WriteFrom(const char *in, uint16_t size)
{
    return WriteDecorator([this, &in, &size](){
        std::memcpy(m_Writer->Buffer, in, size);
        m_Writer->Size = size;
    });
}

template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::WriteFrom(std::istream& in_stream, uint16_t size)
{
    return WriteDecorator([this, &in_stream, &size](){
        in_stream.read(m_Writer->Buffer, size);
        
        m_Writer->Size = size;
    });
}

template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::WriteFrom(UserWriteCallback callback)
{
    return WriteDecorator([this, callback](){
        callback(m_Writer->Buffer, m_Writer->Size);
    });
}

template<size_t msg_size>
template<typename M, typename ...Args>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::WriteEmplace(Args&& ...args)
{
    return WriteDecorator([this, args...]{
        m_Writer->Size = sizeof(M);
        new (m_Writer->Buffer) M(args...);
    });
}

template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::ReadDecorator(const std::function<void(void)> read_impl)
{
    m_num_of_waiting_readers.fetch_add(1, std::memory_order_relaxed);

    // Wait
    {
        auto lock_ = std::unique_lock<std::mutex>(m_ReaderMutex);

        m_ReadCV.wait(lock_, [this]{ return isReadAvailable() || m_ShutDownReaders; });

        if (m_ShutDownReaders)
        {
            while(isReadAvailable())
            {
                ReadCriticalSection(read_impl); // complete remaining reads before shutdown 
            }

            m_num_of_waiting_readers.fetch_sub(1, std::memory_order_relaxed);

            return OperationStatus::SHUTDOWN;
        }
    }
        
    OperationStatus op_status;
    {   
        auto lock_ = std::unique_lock<std::mutex>(m_ReaderMutex);
        op_status = ReadCriticalSection(read_impl);
    }
    
    m_num_of_waiting_readers.fetch_sub(1, std::memory_order_relaxed);

    return op_status;
} // ReadDecorator

template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::ReadCriticalSection(const std::function<void(void)> &read_impl)
{
    if (isReadAvailable())
    {
        if (m_Reader == m_Messages.cend())
        {
            m_Reader = m_Messages.cbegin();
        }

        read_impl();
        m_Reader = std::next(m_Reader);

        messages_available_for_reading.fetch_sub(1, std::memory_order_relaxed);

        return OperationStatus::SUCCESS;
    }

    return OperationStatus::FAIL;
}

template <size_t msg_size>
LogQueue<msg_size>::OperationStatus LogQueue<msg_size>::WriteDecorator(std::function<void(void)> write_impl)
{
    for(;;) {
    // Wait until write available
    writer_spinlock.lock([this]{ return isWriteAvailable() || m_ShutDownWriters; });
    
    if (m_ShutDownWriters)
    {   
        writer_spinlock.unlock();
        return OperationStatus::SHUTDOWN;
    }

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
    return OperationStatus::SUCCESS;
}

template <size_t msg_size>
bool LogQueue<msg_size>::isAlive() const noexcept
{ 
    return m_Alive;
}

template <size_t msg_size>
void LogQueue<msg_size>::ShutDownReaders() 
{ 
    m_ShutDownReaders = true;

    while(m_num_of_waiting_readers.load(std::memory_order_relaxed))
    {
        m_ReadCV.notify_all(); 
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

template <size_t msg_size>
void LogQueue<msg_size>::ShutDownWriters() noexcept
{ 
    m_ShutDownWriters = true;
}

template <size_t msg_size>
void LogQueue<msg_size>::ShutDown() 
{ 
    ShutDownWriters();
    ShutDownReaders();

    m_Alive = false;
}

template <size_t msg_size>
bool LogQueue<msg_size>::isReadAvailable() const noexcept
{
    return GetMessagesCount() > 0; 
}

template <size_t msg_size>
bool LogQueue<msg_size>::isWriteAvailable() const noexcept
{
    return GetMessagesCount() < m_QueueSize;
}

template <size_t msg_size>
size_t LogQueue<msg_size>::GetMessagesCount() const noexcept 
{
    return messages_available_for_reading.load(std::memory_order_relaxed);
}

template <size_t msg_size>
size_t LogQueue<msg_size>::GetSize() const noexcept
{
    return m_QueueSize;
}

} // namespace obps