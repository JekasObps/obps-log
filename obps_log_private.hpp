#pragma once

#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>


#include "ObpsLogConfig.hpp"
#include "log_base.hpp"

namespace obps
{

class Log final : public LogBase 
{
public:
    explicit Log(const LogSpecs& specs);
    ~Log();

    void AddOutput(const LogSpecs::OutputSpec & o_spec);
    
    template <typename ...Args>
    void Write(LogLevel level, Args ...args);
private:
    using LogThreadFunction = LoggerThreadStatus_ (LogQueueSptr, std::shared_ptr<std::ostream> output);
    static LogThreadFunction LogThread;

    using Output = std::tuple<
        const LogLevel, // severity level of the output target 
        const LogSpecs::OutputModifier, // isolate specific level   
        LogQueueSptr, // output specific queue
        FormatSignature*, // corresponding formatter 
        std::shared_ptr<std::ostream>
    >;

    template <typename ...Args>
    MessageData BuildMessage(LogLevel level, FormatSignature* format, Args ...args);

    static Output CreateOutput(const LogSpecs::OutputSpec & o_spec);

    std::vector<Output> m_Outputs;
    LogPoolSptr m_Pool;
};

template <typename ...Args>
void Log::Write(LogLevel level, Args ...args)
{
    for(auto && [lvl, mod, que, fmt, out] : m_Outputs)
    {
        if (lvl >= level) // if level is relevant for current output
        {
            que->WriteEmplace<MessageData>(std::move(BuildMessage(level, fmt, args...)));
        }
    }
}

template <typename ...Args>
Log::MessageData Log::BuildMessage(LogLevel level, FormatSignature* format, Args ...args)
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
        text.c_str()
    };

    return message_data;
}

} // namespace obps

namespace 
{
    using LogLevel = obps::LogLevel;
}