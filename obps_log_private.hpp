#pragma once

#include "ObpsLogConfig.hpp"

#include <thread>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <atomic>


#include "log_base.hpp"
#include "log_queue.hpp"
#include "thread_pool.hpp"
#include "log_scope_destroyer.hpp"

namespace obps
{

using namespace std::chrono_literals;

class Log final : public LogBase 
{
public:
    using LogOnExcept = LoggerThreadStatus_ (const std::exception& e);
    using LogThreadFunction = LoggerThreadStatus_ ();

    Log& Attach(Log& other_log);

    bool HasAttachedLog() const noexcept;

    template <typename ...Args>
    void Write(LogLevel level, Args ...args);
    

    explicit Log(const LogSpecs& specs);
    ~Log();
    
private:
    explicit Log(std::unique_ptr<std::ostream> stream, 
        LogLevel level, 
        size_t queue_size, 
        Formatter formatter, 
        Log* attach, 
        LogPoolSptr pool
        );

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    Log(Log&&) = delete;
    Log& operator=(Log&&) = delete;
    
    static std::unique_ptr<std::ostream> OpenFileStream(const std::string& logname);
    bool IsRelevantLevel(LogLevel level) const noexcept;

    template <typename ...Args>
    void WriteForward(LogLevel level, Args ...args);

    void WriteForward(LogLevel level, const std::string& message);
    
    void SendToQueue(const std::string& message);

    template <typename ...Args>
    std::string BuildMessage(LogLevel level, Args ...args); 

    static LogOnExcept LogExceptHandler;
    LogThreadFunction  LogThread;

    std::unique_ptr<std::ostream> m_Output;
    LogLevel m_Level;
    LogQueue<MAX_MSG_SIZE> m_Queue;

    Formatter m_Format;

    LogPoolSptr m_Pool;
    Log* m_AttachedLog = nullptr;
};

template <typename ...Args>
void Log::Write(LogLevel level, Args ...args)
{
    WriteForward(level, args...);
}

template <typename ...Args>
void Log::WriteForward(LogLevel level, Args ...args)
{
    if (IsRelevantLevel(level))
    {
        std::string message = BuildMessage(level, args...);
        SendToQueue(message);
    }

    if (HasAttachedLog())
    {
        m_AttachedLog->WriteForward(level, args...);
    }
}

template <typename ...Args>
std::string Log::BuildMessage(LogLevel level, Args ...args)
{
    std::stringstream helper_stream;
    std::string content, message;

	(helper_stream << ... << args);
    std::getline(helper_stream, content, '\0');

    helper_stream.clear();

    auto&& message_data = MessageData{ 
        std::move(content), 
        "%F %T", 
        PrettyLevel(level), 
        std::this_thread::get_id() 
    };

    m_Format(helper_stream, message_data);

    std::getline(helper_stream, message, '\0');
    return message;
}

} // namespace obps
