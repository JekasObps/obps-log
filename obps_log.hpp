#pragma once

#include <thread>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "ObpsLogConfig.hpp"
#include "log_queue.hpp"

#ifndef DEFAULT_QUEUE_SIZE
#   define DEFAULT_QUEUE_SIZE 16
#endif

#ifndef MAX_MSG_SIZE
#   define MAX_MSG_SIZE 256
#endif

#ifndef POLLING_MICROS_DELAY
#   define POLLING_MICROS_DELAY 500
#endif

namespace obps
{

enum class LogLevel 
{ 
    ERROR,
    WARN,
    INFO,
    DEBUG 
};

constexpr auto PrettyLevel(const LogLevel level)
{
    switch (level)
    {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::DEBUG: return "DEBUG";
        default: 
            return "UnknownLevel";
    }
}

class ObpsLog
{
public:
    static std::shared_ptr<ObpsLog> CreateLog(const std::string& logname, LogLevel level);
    static std::shared_ptr<ObpsLog> CreateLog(std::ostream& out, LogLevel level);
    
    void Attach(std::shared_ptr<ObpsLog> other_log);

    template <typename ...Args>
    void Log(LogLevel level, Args ...args);
    
    ~ObpsLog();
    
    // FORMATTING--:
    using FormatSignature = void (
        std::ostream&       msg,
        const std::string&  content,
        const std::string&  date_fmt,
        const std::string&  pretty_level,
        std::thread::id     tid
    );

    using Formatter = std::function<FormatSignature>;

    static FormatSignature default_format;
    static FormatSignature JSON_format;

    // :--FORMATTING //
    static std::string GetTimeStr(const std::string& fmt);

    ObpsLog(const ObpsLog&) = delete;
    ObpsLog(ObpsLog&&) = delete;
    ObpsLog& operator=(const ObpsLog&) = delete;
    ObpsLog& operator=(ObpsLog&&) = delete;

private:
    explicit ObpsLog(
        std::unique_ptr<std::ostream> stream, 
        LogLevel level = LogLevel::WARN, 
        size_t queue_size = DEFAULT_QUEUE_SIZE,
        Formatter formatter = default_format
    );
    
    static std::string MakeLogFileName(const std::string& prefix_name);

    void WriterFunction();

#ifdef LOG_ON
    std::unique_ptr<std::ostream> m_Output;
    LogLevel m_Level;
    LogQueue<MAX_MSG_SIZE, POLLING_MICROS_DELAY> m_Queue;
    std::thread m_LogWriter;
    Formatter m_Format;

    std::weak_ptr<ObpsLog> m_AttachedLog;

#endif // LOG_ON
};

template <typename ...Args>
void ObpsLog::Log(LogLevel level, Args ...args)
{
#ifdef LOG_ON
    if (m_Level < level) { return; }

    std::string content;
    std::stringstream args_stream;
	(args_stream << ... << args);
    std::getline(args_stream, content, '\0');

    std::stringstream message_stream;
    m_Format (message_stream, 
        std::move(content), 
        "%F %T", 
        PrettyLevel(level), 
        std::this_thread::get_id()
        );

    std::string msg;
    std::getline(message_stream, msg, '\0');
    m_Queue.Write(msg.c_str(), msg.size());

    if (!m_AttachedLog.expired())
    {
        m_AttachedLog.lock()->m_Queue.Write(msg.c_str(), msg.size());
    }

#endif // LOG_ON
}

} // namespace obps
