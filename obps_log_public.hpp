#pragma once

#include "ObpsLogConfig.hpp"

#ifdef LOG_ON
    #include "obps_log_private.hpp"

    #define CONCAT(a, b) a ## b
    #define EXP(line, suf) CONCAT(line, suf)

    #define _GLOBAL_LOG_ID __obps_global_log
    #define GLOBAL_LOG 

    #define _SCOPE_LOG_ID __obps_scope_log
    
    #define SCOPE_LOG(level, file_name_or_stream, ...) \
        static auto _SCOPE_LOG_ID = obps::Log({file_name_or_stream, obps::LogLevel::level, __VA_ARGS__})
    
    #define ATTACH(level, file_name_or_stream, ...) \
        static auto EXP(__attached_log_, __LINE__) = obps::Log({file_name_or_stream, obps::LogLevel::level, __VA_ARGS__}); \
        _SCOPE_LOG_ID.Attach( EXP(__attached_log_, __LINE__) )

#else
    #define SCOPE_LOG(...) ;
    #define ATTACH(...) ;
#endif // LOG_ON