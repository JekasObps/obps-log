#pragma once

#include <iostream>

#include "ObpsLogConfig.hpp"
#include "thread_pool.hpp"


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
        const std::string&  Content;
        const std::string&  DateFmt;
        const std::string&  PrettyLevel;
        std::thread::id     Tid;
    };

public:
    using FormatSignature = void (std::ostream&, MessageData);
    using Formatter = std::function<FormatSignature>;
    
    static FormatSignature default_format;
    static FormatSignature JSON_format;

    

    static std::string GetTimeStr(const std::string& fmt)
    {
        const auto date = std::chrono::system_clock::now();
        const std::time_t t_c = std::chrono::system_clock::to_time_t(date);
        tm date_info;

        __localtime(&date_info, &t_c);
        
        char timestr_buffer[128];
        strftime(timestr_buffer, 128, fmt.c_str(), &date_info);

        return timestr_buffer;
    }

    static std::string MakeLogFileName(const std::string& prefix_name)
    {
        return prefix_name + "-" + GetTimeStr("%F") + ".log";
    }

    enum class LoggerThreadStatus_ {RUNNING, FINISHED, FAILED};
    
    using LogPool = ThreadPool<LoggerThreadStatus_, std::exception, LoggerThreadStatus_::FINISHED, LoggerThreadStatus_::FAILED>;
    using LogPoolSptr = std::shared_ptr<LogPool>;

protected:
    using LogExceptHandler = std::function<void(std::exception& e)>;


    struct LogSpecs
    {
        enum TargetType {FILENAME, STREAM};
        
        struct FilenameOrStream
        {
            FilenameOrStream(){}
            FilenameOrStream(const char* filename)
              : Type(TargetType::FILENAME) { Value.Filename = filename; }
            FilenameOrStream(std::ostream& stream)
              : Type(TargetType::STREAM) { Value.Stream = &stream; }
              
            union 
            {
                const char*   Filename;
                std::ostream* Stream;
            } Value;

            TargetType Type;
        };
        
        LogSpecs& Target(FilenameOrStream target) 
        { 
            _Target = target;
            return *this;
        }

        LogSpecs& Level(LogLevel level)
        {
            _Level = level;
            return *this;
        }

        LogSpecs& Format(Formatter f)
        {
            _Format = f;
            return *this;
        }

        LogSpecs& ExceptionHandler(LogExceptHandler h)
        {
            _Handler = h;
            return *this;
        }

        LogSpecs& QueueSize(size_t size)
        {
            _QSize = size;
            return *this;
        } 
        
        LogSpecs& Attached(LogBase& log)
        {
            _Attached = &log;
            return *this;
        }

        LogSpecs& ThreadPool(LogPoolSptr pool)
        {
            _ThreadPool = pool;
            return *this;
        }

        FilenameOrStream _Target   = std::cout;
        LogLevel         _Level    = LogLevel::WARN;
        Formatter        _Format   = default_format;
        LogExceptHandler _Handler  = nullptr;
        size_t           _QSize    = DEFAULT_QUEUE_SIZE;
        LogBase*         _Attached = nullptr;
        LogPoolSptr     _ThreadPool = LogPool::GetInstance();

    }; // class LogSpecs
}; // class LogBase


} // namespace obps