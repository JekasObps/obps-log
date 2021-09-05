#pragma once

#include <iostream>
#include <set>

#include "ObpsLogConfig.hpp"
#include "thread_pool.hpp"
#include "log_queue.hpp"


#if defined(WIN32)
#    define __localtime(x, y) localtime_s( x, y )
#elif defined(LINUX)
#    define __localtime(x, y) localtime_r( y, x )
#endif

namespace obps
{

enum class LogLevel {OBPS_LOG_LEVELS};

constexpr auto PrettyLevel(const LogLevel level)
{
    switch (level)
    {
        OBPS_LOG_PRETTY_LEVELS; // !important semi-column
        // generates: 
        //    case LogLevel::<UserDefinedLevel>: return "<UserDefinedLevel>";
        default: 
            return "UnknownLevel";
    }
}

class LogBase
{
protected:
    struct MessageData
    {
        const std::time_t       TimeStamp;
        const LogLevel          Level;
        const std::thread::id   Tid;
        const char              Text[1]; // flex array
    };

    LogBase(){}
    ~LogBase(){}
public:
    enum class LoggerThreadStatus_ 
    {
        RUNNING,    // logger thread hasn't finished yet
        FINISHED,   // logger thread has finished
        ABORTED     // logger thread has aborted
    };

    using FormatSignature = void (std::ostream&, MessageData);
    using Formatter = std::function<FormatSignature>;
    
    static FormatSignature default_format;
    static FormatSignature JSON_format;

    static std::unique_ptr<std::ostream> OpenFileStream(const std::string& logname);
    static std::string MakeLogFileName(const std::string& prefix_name);
    static std::string GetTimeStr(const std::string& fmt);

    using LogPool = ThreadPool<LoggerThreadStatus_, LoggerThreadStatus_::RUNNING, LoggerThreadStatus_::FINISHED, LoggerThreadStatus_::ABORTED>;
    using LogPoolSptr = std::shared_ptr<LogPool>;
    using LogExceptHandler = std::function<void(std::exception& e)>;

    using LogQueueSptr = std::shared_ptr<LogQueue<DEFAULT_QUEUE_SIZE>>;

    LogBase(const LogBase&) = delete;
    LogBase& operator=(const LogBase&) = delete;
    LogBase(LogBase&&) = delete;
    LogBase& operator=(LogBase&&) = delete;
    
    struct LogSpecs
    {
        enum class OutputType {FILENAME, STREAM};
        enum class OutputModifier {NONE, ISOLATED};

        struct FilenameOrStream
        {
            FilenameOrStream()
            {}

            FilenameOrStream(const char* filename)
              : Type(OutputType::FILENAME) 
            {
                Value.Filename = filename; 
            }

            FilenameOrStream(std::ostream& stream)
              : Type(OutputType::STREAM) 
            {
                Value.Stream = &stream; 
            }
            
            // custom variant pattern
            union 
            {
                const char*   Filename;
                std::ostream* Stream;
            } Value;

            OutputType Type;
        };

        using OutputSpec = std::tuple<
            LogLevel,                     // severity level of the output target 
            OutputModifier,               // isolate specific level   
            FilenameOrStream,             // output
            LogQueueSptr,                 // specific queue
            Formatter                     // corresponding formatter 
        >;
        
        LogSpecs& Outputs(std::initializer_list<OutputSpec> l);
        LogSpecs& ThreadPool(LogPoolSptr pool);
        LogSpecs& DefaultQueue(LogQueueSptr queue);

        LogPoolSptr _ThreadPool { LogPool::GetInstance() };

        LogQueueSptr _DefaultLogQueue {
            std::make_shared<LogQueue<DEFAULT_QUEUE_SIZE>>()
        };

        std::set<OutputSpec> _Outputs {
            {
                LogLevel::WARN, 
                OutputModifier::NONE,
                std::cout,
                _DefaultLogQueue,
                default_format
            }
        };
    }; // class LogSpecs

}; // class LogBase


} // namespace obps