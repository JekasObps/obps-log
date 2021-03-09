#pragma once

#include <thread>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

#include "ObpsLogConfig.hpp"
#include "log_queue.hpp"

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

class Log
{
public:
    static std::shared_ptr<Log> Create(const std::string& logname, LogLevel level);
    static std::shared_ptr<Log> Create(std::ostream& out, LogLevel level);
    
    void Attach(std::shared_ptr<Log> other_log);

    template <typename ...Args>
    void Write(LogLevel level, Args ...args);
    
    ~Log();
    
    // FORMATTING--:
    struct MessageData
    {
        const std::string&  Content;
        const std::string&  DateFmt;
        const std::string&  PrettyLevel;
        std::thread::id     Tid;
    };

    using FormatSignature = void (std::ostream&, MessageData);
    using Formatter = std::function<FormatSignature>;

    static FormatSignature default_format;
    static FormatSignature JSON_format;

    // :--FORMATTING //
    static std::string GetTimeStr(const std::string& fmt);

    Log(const Log&) = delete;
    Log(Log&&) = delete;
    Log& operator=(const Log&) = delete;
    Log& operator=(Log&&) = delete;

private:
    explicit Log(
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

    std::weak_ptr<Log> m_AttachedLog;

#endif // LOG_ON
};

template <typename ...Args>
void Log::Write(LogLevel level, Args ...args)
{
#ifdef LOG_ON
    if (m_Level < level) { return; }

    std::string content;
    std::stringstream helper_stream;
	(helper_stream << ... << args);
    std::getline(helper_stream, content, '\0');

    helper_stream.clear();

    m_Format (helper_stream, 
        std::move(content), 
        "%F %T", 
        PrettyLevel(level), 
        std::this_thread::get_id()
        );

    std::string msg;
    std::getline(helper_stream, msg, '\0');
    m_Queue.Write(msg.c_str(), msg.size());

    if (!m_AttachedLog.expired())
    {
        m_AttachedLog.lock()->m_Queue.Write(msg.c_str(), msg.size());
    }

#endif // LOG_ON
}

} // namespace obps
