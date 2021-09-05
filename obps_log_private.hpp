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
    template <typename ...Args>
    void Write(LogLevel level, Args ...args);
    
    explicit Log(const LogSpecs& specs);
    ~Log();
private:
    using LogThreadFunction = LoggerThreadStatus_ (std::ostream &output);
    
    void SendToQueue(const std::string& message);

    template <typename ...Args>
    std::string BuildMessage(LogLevel level, Args ...args); 

    LogThreadFunction LogThread;

    using Output_ = std::tuple<
        LogLevel,       // severity level of the output target 
        LogSpecs::OutputModifier,
        std::ostream,   // output stream
        Formatter       // corresponding formatter 
    >;

    std::vector<Output_> m_Outputs;
    
    std::shared_ptr<LogQueue<MAX_MSG_SIZE>> m_Queue;
    
    LogPoolSptr m_Pool;
};

template <typename ...Args>
void Log::Write(LogLevel level, Args ...args)
{
    
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
