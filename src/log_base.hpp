#pragma once

#include <cstdlib>

#include <iostream>
#include <set>


#include "ObpsLogConfig.hpp"
#include "thread_pool.hpp"
#include "ring_queue.hpp"


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
public:
    static constexpr size_t text_field_size = 256;
    static constexpr size_t default_queue_size = DEFAULT_QUEUE_SIZE;
    using FormatSignature = void (std::ostream&, const std::time_t, const LogLevel, const std::thread::id, const char* text);
    using Formatter = std::function<FormatSignature>;

protected:
    struct MessageData
    {
        std::time_t             TimeStamp;
        LogLevel                Level;
        std::thread::id         Tid;
        FormatSignature*        Format;
        char                    Text[text_field_size];

        MessageData(){};
        
        MessageData(const std::time_t ts, const LogLevel lvl, const std::thread::id tid, FormatSignature* const fmt, const char * const src) 
          : TimeStamp(ts)
          , Level(lvl)
          , Tid(tid)
          , Format(fmt)
        {
            std::memcpy(Text, src, text_field_size);
        }

        MessageData(const MessageData& other) 
          : TimeStamp(other.TimeStamp)
          , Level(other.Level)
          , Tid(other.Tid)
          , Format(other.Format)
        {
            std::memcpy(Text, other.Text, text_field_size);
        }
    }; // struct MessageData

    LogBase(){}
    ~LogBase(){}

public:
    enum class LoggerThreadStatus_ 
    {
        RUNNING,    // logger thread hasn't finished yet
        FINISHED,   // logger thread has finished
        ABORTED     // logger thread has aborted
    };

    static std::unique_ptr<std::ostream> OpenFileStream(const std::string& logname);

    using LogPool = ThreadPool<LoggerThreadStatus_, LoggerThreadStatus_::RUNNING, LoggerThreadStatus_::FINISHED, LoggerThreadStatus_::ABORTED>;
    using LogPoolSptr = std::shared_ptr<LogPool>;

    static LogPoolSptr GetDefaultThreadPoolInstance();

    using LogQueue_ = RingQueue<MAX_MSG_SIZE>; 
    using LogQueueSptr = std::shared_ptr<LogQueue_>;
    
    static LogQueueSptr GetDefaultQueueInstance();

    static FormatSignature default_format;
    static FormatSignature JSON;

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
            union {
                const char*   Filename;
                std::ostream* Stream;
            } Value;

            OutputType Type;
        }; // struct FilenameOrStream

        struct OutputSpec
        {
            LogLevel level;                     
            FilenameOrStream file_or_stream;    
            // defaults:   
            OutputModifier mod = OutputModifier::NONE;
            LogQueueSptr queue = GetDefaultQueueInstance();               
            FormatSignature* format = &LogBase::default_format;
        };

        LogSpecs(std::initializer_list<OutputSpec> outputs, LogPoolSptr pool = GetDefaultThreadPoolInstance())
          : _Outputs(outputs), _ThreadPool(pool)
        {}

        std::vector<OutputSpec> _Outputs;
        LogPoolSptr             _ThreadPool;
    }; // class LogSpecs

}; // class LogBase

std::string MakeLogFileName(const std::string& prefix_name);
std::string GetTimeStr(const char* fmt, const std::time_t stamp) noexcept;
const std::time_t get_timestamp() noexcept;


} // namespace obps