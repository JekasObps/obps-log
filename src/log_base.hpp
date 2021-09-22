#pragma once



#include <variant> // std::variant
#include <filesystem> // std::filesystem::path
#include <vector> // std::vector
#include <ostream> // std::ostream

#include "log_def.hpp"
#include "log_registry.hpp"

#if defined(WIN32)
#    define __localtime(x, y) localtime_s( x, y )
#elif defined(LINUX)
#    define __localtime(x, y) localtime_r( y, x )
#endif

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
namespace fs = std::filesystem;

class LogBase
{
public:
    using FormatFunctionPtr = MessageData::FormatFunctionPtr;
protected:
    LogBase() = default;
    ~LogBase() = default;

public:
    static std::unique_ptr<std::ostream> OpenFileStream(fs::path log_path);

    static MessageData::FormatFunction default_format;
    static MessageData::FormatFunction JSON;

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