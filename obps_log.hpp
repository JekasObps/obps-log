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

using namespace std::chrono_literals;

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

class Log final
{
public:
    static std::shared_ptr<Log> CreateRef(const std::string& logname, LogLevel level);
    static std::shared_ptr<Log> CreateRef(std::ostream& out, LogLevel level);
    
    static Log Create(const std::string& logname, LogLevel level);
    static Log Create(std::ostream& out, LogLevel level);
    
    Log& Attach(Log& other_log);

    bool HasAttachedLog() const;

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
    
    // bool CheckNeedInMessageComposing(LogLevel level) const;
    bool IsRelevantLevel(LogLevel level) const;

    template <typename ...Args>
    void WriteForward(LogLevel level, Args ...args);

    void WriteForward(LogLevel level, const std::string& message);
    
    void SendToQueue(const std::string& message);

    template <typename ...Args>
    std::string BuildMessage(LogLevel level, Args ...args);

    static std::string MakeLogFileName(const std::string& prefix_name);

    void WriterFunction();

#ifdef LOG_ON
    std::unique_ptr<std::ostream> m_Output;
    LogLevel m_Level;
    LogQueue<MAX_MSG_SIZE, POLLING_MICROS_DELAY> m_Queue;
    std::thread m_LogWriter;
    Formatter m_Format;

    Log* m_AttachedLog = nullptr;
#endif // LOG_ON
};

template <typename ...Args>
void Log::Write(LogLevel level, Args ...args)
{
#ifdef LOG_ON
    WriteForward(level, args...);
#endif // LOG_ON
}

template <typename ...Args>
void Log::WriteForward(LogLevel level, Args ...args)
{
    if (IsRelevantLevel(level))
    {
        std::string message = BuildMessage(level, args...);
        SendToQueue(message);

        if (HasAttachedLog())
            m_AttachedLog->WriteForward(level, message);

        return;
    }

    if (HasAttachedLog())
        m_AttachedLog->WriteForward(level, args...);
}

template <typename ...Args>
std::string Log::BuildMessage(LogLevel level, Args ...args)
{
    std::stringstream helper_stream;
    std::string content, message;

	(helper_stream << ... << args);
    std::getline(helper_stream, content, '\0');

    helper_stream.clear();

    m_Format(helper_stream, { std::move(content), "%F %T", 
                              PrettyLevel(level), std::this_thread::get_id() });

    std::getline(helper_stream, message, '\0');
    return message;
}

} // namespace obps
