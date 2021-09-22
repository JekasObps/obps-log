#pragma once

#include <unordered_set> // std::unordered_set
#include <set> // std::set
#include <sstream> // std::stringstream
#include <string> // std::string

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

// Checks message relevance to log's output targets by comparing levels and checking MutedLevels
// then construct and write one message per relevant output.
// Each message constructed from scratch using unique format per output 
// and written into an output specific queue.
template <typename ...Args>
void Log::Write(LogLevel level, bool sync, Args ...args)
{
    for(auto && [lvl, mod, que, fmt, out] : m_Outputs)
    {
        if (lvl >= level && (! m_MutedLevels.contains(level)))
        {
            que->WriteEmplace<MessageData>(std::move(BuildMessage(level, fmt, sync, args...)));
        }
    }
}

// Helper that parses user arguments and creates MessageData that will be passed into a output's queue.
//
// Params:
//  LogLevel level:             message level to be displayed in log.
//  FormatFunctionPtr format:   format function that will be used for final message composing before write.
//  bool sync:                  flag that indicates whenever need to flush output stream after mesasge writing.
//  Args ...args:               any args that user provide that will become part of a message.
//
// Return: 
//  MessageData:                struct that will be moved into a output's queue
template <typename ...Args>
MessageData Log::BuildMessage(LogLevel level, FormatFunctionPtr format, bool sync, Args ...args)
{
    std::stringstream serializer;
    std::string text;

	(serializer << ... << args);
    std::getline(serializer, text, '\0');

    assert(text.size() <= MessageData::text_field_size);

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