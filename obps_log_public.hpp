#pragma once

#include "ObpsLogConfig.hpp"

#ifdef LOG_ON
    #include "obps_log_private.hpp"

    #define CONCAT(a, b) a ## b
    #define EXP(line, suf) CONCAT(line, suf)

    #define _GLOBAL_LOG_ID __obps_global_log
    #define _SCOPE_LOG_ID __obps_scope_log
    #define _LOG_DESTROYER_ID __obps_log_destroyer

    /*
    *   Register propper log shutdown. 
    *   User must place this macro at the beginning of the main function.
    *   or another scope that will limit log opperation.
    */
    #define OBPS_LOG_INIT() obps::LogScopeDestroyer _LOG_DESTROYER_ID;
    
    /*
    *
    */
    #define GLOBAL_LOG()

    /*
    *   Call at the beginning of the logging scope
    */
    #define SCOPE_LOG(level, file_name_or_stream, ...) \
        static auto _SCOPE_LOG_ID = obps::Log({file_name_or_stream, obps::LogLevel::level, __VA_ARGS__})
    
    /*
    *   Attach another log to the scope log.
    */
    #define ATTACH(level, file_name_or_stream, ...) \
        static auto EXP(__attached_log_, __LINE__) = obps::Log({file_name_or_stream, obps::LogLevel::level, __VA_ARGS__}); \
        _SCOPE_LOG_ID.Attach( EXP(__attached_log_, __LINE__) )

#else
    #define SCOPE_LOG(...) ;
    #define ATTACH(...) ;
#endif // LOG_ON