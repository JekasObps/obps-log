#include "log_registry.hpp"

namespace obps
{

LogRegistry::LogQueueSptr LogRegistry::CreateAndGetQueue(const std::string id, const size_t size)
{
    auto&& [iter, emplaced] = m_Queues.try_emplace(id, std::make_shared<LogQueue_>(size));
    if ((!emplaced) && iter->second->GetSize() != size)
    {
        throw std::logic_error("Trying to create queue of different size with same id!");
    }

    return iter->second;
} 

void LogRegistry::WipeAllQueues()
{
    for(auto && [_, queue] : m_Queues)
    {
        queue->ShutDown();
    }

    m_Queues.clear();
}

// Generates queue id
//
//
std::string LogRegistry::GenerateQueueUid()
{
    static std::atomic<size_t> count = {0};
    return std::format("q_{:#}", count.fetch_add(1, std::memory_order_relaxed));
}

} // namespace obps