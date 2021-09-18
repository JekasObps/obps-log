#include "log_registry.hpp"

namespace obps
{
template class LogRegistry::LogPool; // instantiate LogPool

LogRegistry::LogQueueSptr LogRegistry::CreateAndGetQueue(const std::string id, const size_t size)
{
    auto&& [iter, emplaced] = m_Queues.try_emplace(id, std::make_shared<LogQueue>(size));
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

// Generates queue id consisting of prefix "q_" and hex incrementor.
// NOTE{Jekas}: thoughts about making it to use thread id of a log writer thread for the uniqueness, or some uid library
//  for now atomic counter for thread safeness is enough.
// 
std::string LogRegistry::GenerateQueueUid()
{
    static std::atomic<size_t> count = {0};
    return std::format("q_{:#}", count.fetch_add(1, std::memory_order_relaxed));
}

void LogRegistry::ObpsLogShutdown()
{
    GetDefaultQueueInstance()->ShutDown();
    GetLogRegistry()->WipeAllQueues();
    GetDefaultThreadPoolInstance()->ShutDown();
}

/*
*   Singleton Builder For ThreadPool.
*/
LogRegistry::LogPoolSptr LogRegistry::GetDefaultThreadPoolInstance() 
{
    static LogPoolSptr s_Instance;
    static bool isInit = false;
    if (!isInit)
    {
        s_Instance = std::make_shared<LogPool>();
        isInit = true;
    }
    return s_Instance;
}

/*
*   Singleton Builder For Queue.
*/
LogRegistry::LogQueueSptr LogRegistry::GetDefaultQueueInstance() 
{
    static LogQueueSptr s_Instance;
    static bool isInit = false;
    if (!isInit)
    {
        s_Instance = std::make_shared<LogQueue>(default_queue_size);
        isInit = true;
    }
    return s_Instance;
}

/*
*   Singleton Builder For LogRegistry.
*/
LogRegistry::LogRegistrySptr LogRegistry::GetLogRegistry()
{
    static LogRegistrySptr s_Instance;
    static bool isInit = false;
    if (!isInit)
    {
        s_Instance = std::make_shared<LogRegistry>();
        isInit = true;
    }
    return s_Instance;
}

} // namespace obps