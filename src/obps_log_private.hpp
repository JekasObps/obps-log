#pragma once

#include <unordered_set> // std::unordered_set
#include <set> // std::set

#include "ObpsLogConfig.hpp"
#include "log_base.hpp"

namespace obps
{

class Log final : public LogBase 
{
public:
    explicit Log(LogSpecs&& specs);
    ~Log() = default;

    void AddOutput(const LogSpecs::OutputSpecs& o_spec);

    template <typename ...Args>
    void Write(LogLevel level, bool sync, Args ...args);

    void Mute(const std::unordered_set<LogLevel>& mute_levels);
    void Unmute(const std::set<LogLevel>& unmute_levels);
private:
    using OstreamSptr = std::shared_ptr<std::ostream>;
    using LogThreadFunction = LoggerThreadStatus (LogQueueSptr, OstreamSptr);
    static LoggerThreadStatus LogThread(LogQueueSptr, OstreamSptr output);
    
    using Output = std::tuple<
        const LogLevel, // severity level of the output target 
        const LogSpecs::OutputModifier, // isolate specific level   
        LogQueueSptr, // output specific queue
        FormatFunctionPtr, // corresponding formatter 
        OstreamSptr
    >;

    template <typename ...Args>
    static MessageData BuildMessage(LogLevel level, FormatFunctionPtr format, bool sync, Args ...args);

    static Output CreateOutput(const LogSpecs::OutputSpecs& o_spec);

    std::vector<Output> m_Outputs;
    LogPoolSptr m_Pool;
    std::unordered_set<LogLevel> m_MutedLevels;
};

template <typename ...Args>
void Log::Write(LogLevel level, bool sync, Args ...args)
{
    for(auto && [lvl, mod, que, fmt, out] : m_Outputs)
    {
        // A message level is relevant for current output and not muted
        if (lvl >= level && (! m_MutedLevels.contains(level)))
        {
            que->WriteEmplace<MessageData>(std::move(BuildMessage(level, fmt, sync, args...)));
        }
    }
}

template <typename ...Args>
Log::MessageData Log::BuildMessage(LogLevel level, FormatFunctionPtr format, bool sync, Args ...args)
{
    std::stringstream serializer;
    std::string text;

	(serializer << ... << args);
    std::getline(serializer, text, '\0');

    assert(text.size() <= text_field_size);

    //TODO: Check size of the text

    auto&& message_data = MessageData{ 
        get_timestamp(),
        level, 
        std::this_thread::get_id(),
        format,
        text.c_str(),
        sync
    };

    return message_data;
}

} // namespace obps

namespace 
{
    using LogLevel = obps::LogLevel;
}