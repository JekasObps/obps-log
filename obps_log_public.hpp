#pragma once

#ifdef LOG_ON
    #include "obps_log_private.hpp"

    #define CONCAT(a, b) a ## b
    #define EXP(line, suf) CONCAT(line, suf)

    #define _SCOPE_LOG_ID __obps_scope_log
    #define SCOPE_LOG(level, file_name_or_stream, ...) \
        static auto _SCOPE_LOG_ID = obps::Log({file_name_or_stream, obps::LogLevel::##level, __VA_ARGS__})
    
    #define ATTACH(level, file_name_or_stream, ...) \
        static auto EXP(__attached_log_, __LINE__) = obps::Log({file_name_or_stream, obps::LogLevel::##level, __VA_ARGS__}); \
        _SCOPE_LOG_ID.Attach( EXP(__attached_log_, __LINE__) )
    
    #define ERROR(...) _SCOPE_LOG_ID.Write(obps::LogLevel::ERROR, __VA_ARGS__)
    #define WARNING(...) _SCOPE_LOG_ID.Write(obps::LogLevel::WARN, __VA_ARGS__)
    #define INFO(...) _SCOPE_LOG_ID.Write(obps::LogLevel::INFO, __VA_ARGS__)
    
    // to turn debug messages on/off
    #ifdef DEBUG_MODE 
        #define DEBUG(...) SCOPE_LOG.Write(obps::LogLevel::DEBUG, __VA_ARGS__)
    #else
        #define DEBUG(...) ;
    #endif // DEBUG_MODE

#else
    #define SCOPE_LOG(...) ;
    #define ATTACH(...) ;
    #define ERROR(...) ;
    #define WARNING(...) ;
    #define INFO(...) ;
    #define DEBUG(...) ;
#endif // LOG_ON