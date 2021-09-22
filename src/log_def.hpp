////
//  Contains common log defenitions and dependencies to another projects  
////

#pragma once 

#include "ObpsLogConfig.hpp"
#include "ring_queue.hpp"
#include "thread_pool.hpp"
#include "message_data.hpp"

namespace obps
{

// Describes logThread states that will be used by the ThreadPool
//
enum class LoggerThreadStatus 
{
    RUNNING,    // logger thread hasn't finished yet
    FINISHED,   // logger thread has finished
    ABORTED     // logger thread has aborted
};

using LogPool = ThreadPool<
    LoggerThreadStatus,
    LoggerThreadStatus::RUNNING, 
    LoggerThreadStatus::FINISHED,
    LoggerThreadStatus::ABORTED
>;

using LogPoolSptr = std::shared_ptr<LogPool>;

using LogQueue = RingQueue<sizeof(MessageData)>; // queue message size depends on MessageData class size
using LogQueueSptr = std::shared_ptr<LogQueue>;

} // namespace obps
