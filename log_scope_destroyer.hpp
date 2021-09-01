#pragma once
#include <memory>

#include "obps_log_private.hpp"

namespace obps
{

// this class used to finilize thread pools gracefuly!
// only for main scope
class LogScopeDestroyer
{
public:
    // shutdown global thread pool
    ~LogScopeDestroyer()
    {
        Log::LogPool::GetInstance()->ShutDown();
        // + also shutdown user defined pools in the future 
    }

    LogScopeDestroyer()
    {}

    LogScopeDestroyer(const LogScopeDestroyer&) = delete;
    LogScopeDestroyer& operator=(const LogScopeDestroyer&) = delete;
    LogScopeDestroyer(LogScopeDestroyer&&) = delete;
    LogScopeDestroyer& operator=(LogScopeDestroyer&&) = delete;
};

} // namespace obps