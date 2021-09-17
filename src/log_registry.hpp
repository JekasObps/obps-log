#pragma once

#include <atomic>
#include <unordered_map>

#include "ObpsLogConfig.hpp"
#include "ring_queue.hpp"

namespace obps
{

class LogRegistry
{
public:
    using LogQueue_ = RingQueue<MAX_MSG_SIZE>; 
    using LogQueueSptr = std::shared_ptr<LogQueue_>;

    LogQueueSptr CreateAndGetQueue(const std::string id, const size_t size);

    void WipeAllQueues();

    static std::string GenerateQueueUid();

    // Non-copyable
    LogRegistry(const LogRegistry&) = delete;
    LogRegistry& operator=(const LogRegistry&) = delete;
    
    // Non-movable
    LogRegistry(LogRegistry&&) = delete;
    LogRegistry& operator=(LogRegistry&&) = delete;
public:
    LogRegistry();
    ~LogRegistry();
private:
    std::unordered_map<std::string, LogQueueSptr> m_Queues;
};
    
} // namespace obps
