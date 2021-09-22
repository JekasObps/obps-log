#pragma once

#include <unordered_map> // std::unordered_map

#include "log_def.hpp"

namespace obps
{

class LogRegistry final
{
public:
    static constexpr size_t default_queue_size = DEFAULT_QUEUE_SIZE;

    static LogPoolSptr GetDefaultThreadPoolInstance();
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
