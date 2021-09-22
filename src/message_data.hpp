#pragma once

#include <cstdlib> // std::memcpy
#include <thread> // std::thread::id

namespace obps
{

// configuration generated enum of log severity levels: ERROR, ..., DEBUG
// can be customized by user
enum class LogLevel 
{
    OBPS_LOG_LEVELS
};

// returns textual representation of LogLevel enum values 
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

struct MessageData
{
public:
    // format function intarface allows user to provide custom formats to the log targets
    using FormatFunction = void (std::ostream&, const std::time_t, const LogLevel, const std::thread::id, const char* text);
    using FormatFunctionPtr = FormatFunction*;
private:
    friend class Log;

    static constexpr size_t text_field_size = 256;

    std::time_t TimeStamp;
    LogLevel Level;
    std::thread::id Tid;
    FormatFunctionPtr Format;
    char Text[text_field_size];
    bool Sync; // used to enable flushes on write

public:
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

} // namespace obps
