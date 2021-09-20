#pragma once

#include <cstdlib>

#include <variant>
#include <filesystem>
#include <vector>
#include <iostream>

#include "ObpsLogConfig.hpp"
#include "log_registry.hpp"


#if defined(WIN32)
#    define __localtime(x, y) localtime_s( x, y )
#elif defined(LINUX)
#    define __localtime(x, y) localtime_r( y, x )
#endif

namespace obps
{

namespace fs = std::filesystem;

enum class LogLevel {OBPS_LOG_LEVELS};
constexpr auto PrettyLevel(const LogLevel level)
{
    switch (level)
    {
        OBPS_LOG_PRETTY_LEVELS
        // generates: 
        //    case LogLevel::<UserDefinedLevel>: return "<UserDefinedLevel>";
        default: 
            return "UnknownLevel";
    }
}
} // namespace obps

namespace std
{
    template<> struct hash<obps::LogLevel>
    {
        size_t operator() (const obps::LogLevel& level) const noexcept
        {
            return hash<int>{}(static_cast<int>(level));
        }
    };
} // namespace std

namespace obps
{
class LogBase
{
public:
    using LoggerThreadStatus = LogRegistry::LoggerThreadStatus;
    using LogPool = LogRegistry::LogPool;
    using LogPoolSptr = LogRegistry::LogPoolSptr;
    using LogQueue = LogRegistry::LogQueue;
    using LogQueueSptr = LogRegistry::LogQueueSptr;


    static constexpr size_t text_field_size = 256;
    using FormatFunction = void (std::ostream&, const std::time_t, const LogLevel, const std::thread::id, const char* text);
    using FormatFunctionPtr = FormatFunction*;

protected:
    struct MessageData
    {
        std::time_t TimeStamp;
        LogLevel Level;
        std::thread::id Tid;
        FormatFunctionPtr Format;
        char Text[text_field_size];
        bool Sync; // used to enable flushes on write

        MessageData() = default;
        
        MessageData(const std::time_t ts, const LogLevel lvl, const std::thread::id tid, FormatFunctionPtr const fmt, const char * const src, bool sync = false)
          : TimeStamp(ts)
          , Level(lvl)
          , Tid(tid)
          , Format(fmt)
          , Sync(sync)
        {
            std::memcpy(Text, src, text_field_size);
        }

        MessageData(const MessageData& other)
          : TimeStamp(other.TimeStamp)
          , Level(other.Level)
          , Tid(other.Tid)
          , Format(other.Format)
          , Sync(other.Sync)
        {
            std::memcpy(Text, other.Text, text_field_size);
        }
    }; // struct MessageData

    LogBase() = default;
    ~LogBase() = default;

public:
    static std::unique_ptr<std::ostream> OpenFileStream(fs::path log_path);

    static FormatFunction default_format;
    static FormatFunction JSON;

    LogBase(const LogBase&) = delete;
    LogBase& operator=(const LogBase&) = delete;
    LogBase(LogBase&&) = delete;
    LogBase& operator=(LogBase&&) = delete;
    
    class LogSpecs
    {
    public:
        enum class OutputModifier {NONE, ISOLATED};

        class PathOrStream
        {
        public:
            PathOrStream(fs::path path): m_Value(path) {}
            PathOrStream(std::ostream& stream): m_Value(&stream) {}

            bool isPath() const noexcept
            {
                return std::holds_alternative<fs::path>(m_Value);
            }

            bool isStream() const noexcept
            {
                return ! isPath();
            }
            
            auto getPath() const
            {
                return std::get<fs::path>(m_Value);
            }

            auto getStream() const
            {
                return std::get<std::ostream*>(m_Value);
            }
            
            std::variant<fs::path, std::ostream*> m_Value;
        }; // struct PathOrStream

        struct OutputSpecs
        {
            LogLevel Level;                     
            PathOrStream Target;    
  
            OutputModifier Mod;
            LogQueueSptr Queue;               
            FormatFunctionPtr Format;

            OutputSpecs(LogLevel lvl, 
                PathOrStream path_or_stream, 
                const size_t queue_size = LogRegistry::default_queue_size,
                const std::string queue_id = LogRegistry::GenerateQueueUid(),
                OutputModifier m = OutputModifier::NONE,
                FormatFunctionPtr fmt = &LogBase::default_format
                )
              : Level(lvl)
              , Target(path_or_stream)
              , Mod(m)
              , Queue(LogRegistry::GetLogRegistry()->CreateAndGetQueue(queue_id, queue_size))
              , Format(fmt)
              {}
        };

        LogSpecs(std::initializer_list<OutputSpecs> outputs, LogPoolSptr pool = LogRegistry::GetDefaultThreadPoolInstance())
          : m_OutputSpecs(outputs),  m_LogPool(pool)
        {}

        std::vector<OutputSpecs>& GetOutputSpecs() noexcept
        {
            return m_OutputSpecs;
        }

        LogPoolSptr& GetLogPool() noexcept
        {
            return m_LogPool;
        }

    private:
        std::vector<OutputSpecs> m_OutputSpecs;
        LogPoolSptr              m_LogPool;
    }; // class LogSpecs

}; // class LogBase

std::string make_log_filename(const std::string& prefix_name);
std::string get_time_string(const char* fmt, const std::time_t stamp) noexcept;
const std::time_t get_timestamp() noexcept;


} // namespace obps