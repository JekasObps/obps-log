#pragma once

#include <atomic>
#include <unordered_map>

#include "ObpsLogConfig.hpp"
#include "ring_queue.hpp"

#include <iostream> // TODO: 
#include "thread_pool.hpp"



namespace obps
{

class LogRegistry final
{
public:
    static constexpr size_t default_queue_size = DEFAULT_QUEUE_SIZE;
    enum class LoggerThreadStatus 
    {
        RUNNING,    // logger thread hasn't finished yet
        FINISHED,   // logger thread has finished
        ABORTED     // logger thread has aborted
    };

    using LogPool = ThreadPool<LoggerThreadStatus, LoggerThreadStatus::RUNNING, LoggerThreadStatus::FINISHED, LoggerThreadStatus::ABORTED>;
    using LogPoolSptr = std::shared_ptr<LogPool>;

    static LogPoolSptr GetDefaultThreadPoolInstance();

    using LogQueue = RingQueue<MAX_MSG_SIZE>; 
    using LogQueueSptr = std::shared_ptr<LogQueue>;
    static LogQueueSptr GetDefaultQueueInstance();
    
    using LogRegistrySptr = std::shared_ptr<LogRegistry>;
    static LogRegistrySptr GetLogRegistry();

    LogQueueSptr CreateAndGetQueue(const std::string id, const size_t size);
    void WipeAllQueues();

    static std::string GenerateQueueUid();
    static void ObpsLogShutdown();

    // Non-copyable
    LogRegistry(const LogRegistry&) = delete;
    LogRegistry& operator=(const LogRegistry&) = delete;
    
    // Non-movable
    LogRegistry(LogRegistry&&) = delete;
    LogRegistry& operator=(LogRegistry&&) = delete;
public:
    LogRegistry() = default;
    ~LogRegistry() = default;
private:
    std::unordered_map<std::string, LogQueueSptr> m_Queues;
};
    
} // namespace obps
