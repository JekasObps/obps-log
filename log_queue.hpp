#pragma once

#include <cstring>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <assert.h>

#include <fstream>
#include <functional>

namespace obps
{

template <size_t msg_size, size_t polling_micros>
class LogQueue final
{
public:
    explicit LogQueue(size_t queue_size);
    ~LogQueue();

    uint16_t Read(char *out);
    uint16_t ReadToFile(std::ostream& dest);
    uint16_t ReadTo(std::function<void(const char *msg, uint16_t size)> f);

    void Write(const char *in, uint16_t size);

    bool isAlive() const { return !m_Terminate; }
    bool isOpen() const { return !m_Closed; }
    
    void Close() 
    { 
        m_Closed = true; 
        m_ReadCV.notify_one(); 
    }

    bool isReadAvailable() const { return m_Reader != m_Writer; }

    bool isWriteAvailable() const
    { 
        auto n = ( std::next(m_Writer) == m_Messages.end() )
                ? m_Messages.begin() : std::next(m_Writer);

        return n != m_Reader;
    }

    std::unique_lock<std::mutex>&& AcqureLock()
    {
        return std::move(std::unique_lock<std::mutex>(m_ReaderMutex));
    }

private:
    void ReadDecorator(std::function<void(void)> read_impl);
    struct Message
    {
        uint16_t Size;
        char Buffer[msg_size];
    };

    std::vector<Message> m_Messages;
    typename std::vector<Message>::const_iterator m_Reader;
    typename std::vector<Message>::iterator       m_Writer;
    size_t m_QueueSize;

    std::mutex m_ReaderMutex;
    std::condition_variable m_ReadCV;

    std::atomic_flag m_WriteFlag = ATOMIC_FLAG_INIT;

    bool m_Terminate = false;
    bool m_Closed = false;

    static_assert(msg_size > 0, "Message size must be greater than 0!");
    static_assert(msg_size <= 1024, "Max Message size is 1024 bytes!");
};

template <size_t msg_size, size_t polling_micros>
LogQueue<msg_size, polling_micros>::LogQueue(size_t queue_size)
  : m_QueueSize(queue_size)
  , m_Messages(queue_size + 1)
  , m_Reader(m_Messages.cbegin())
  , m_Writer(m_Messages.begin())
{
    assert(queue_size > 0 && "Queue size must be greater than 0!");
}

template <size_t msg_size, size_t polling_micros>
LogQueue<msg_size, polling_micros>::~LogQueue()
{
    m_Terminate = true;
    m_ReadCV.notify_one();
}

template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::ReadDecorator(std::function<void(void)> read_impl)
{
    auto lock = std::unique_lock<std::mutex>(m_ReaderMutex);

    m_ReadCV.wait(lock, [this]{ 
        return isReadAvailable() || m_Terminate || m_Closed; 
    });
    
    if (m_Terminate)
    {
        return;
    }

    if (isReadAvailable())
    {
        read_impl();
        m_Reader = (m_Messages.cend() == std::next(m_Reader))
                 ? m_Messages.cbegin() 
                 : std::next(m_Reader);
    }
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
        dest.write(m_Reader->Buffer, m_Reader->Size); 
        dest.flush();
    });

    return size;
}

template <size_t msg_size, size_t polling_micros>
uint16_t LogQueue<msg_size, polling_micros>::ReadTo(std::function<void(const char *msg, uint16_t size)> f)
{
    size_t size;

    ReadDecorator([this, &f, &size]{
        size = m_Reader->Size;
        f(m_Reader->Buffer, size);
    });

    return size;
}

template <size_t msg_size, size_t polling_micros>
void LogQueue<msg_size, polling_micros>::Write(const char *in, uint16_t size)
{
    while (true)
    {   
        while (!isWriteAvailable() || m_WriteFlag.test_and_set(std::memory_order_acquire))
            std::this_thread::sleep_for(std::chrono::microseconds(polling_micros)); // spin
    
        if (isWriteAvailable())
        {
            std::memcpy(m_Writer->Buffer, in, size);
            m_Writer->Size = size;
            break;
        }
        else
        {
            m_WriteFlag.clear(std::memory_order_release);
        }
    }

    m_Writer = ( m_Messages.end() == std::next(m_Writer) )
               ? m_Messages.begin() : std::next(m_Writer);
    m_ReadCV.notify_one();

    m_WriteFlag.clear(std::memory_order_release);
}

} // namespace obps