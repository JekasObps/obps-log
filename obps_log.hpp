#pragma once

#ifdef LOG_ON
    #include "obps_log_private.hpp"

    #define CONCAT(a, b) a ## b
    #define EXP(line, suf) CONCAT(line, suf)

    #define SCOPE_LOG __obps_scope_log
    #define MakeScopeLog(...) static auto SCOPE_LOG = obps::Log( __VA_ARGS__ );
    
    #define Attach(...) \
        static auto EXP(__attached_log_, __LINE__) = obps::Log(__VA_ARGS__); \
        SCOPE_LOG.Attach( EXP(__attached_log_, __LINE__) )
    
    #define Error(...) SCOPE_LOG.Write(obps::LogLevel::ERROR, __VA_ARGS__ )
    #define Warning(...) SCOPE_LOG.Write(obps::LogLevel::WARN, __VA_ARGS__ )
    #define Info(...) SCOPE_LOG.Write(obps::LogLevel::INFO, __VA_ARGS__ )
    
    // to turn debug messages on/off
    #ifdef DEBUG_MODE 
        #define Debug(...) SCOPE_LOG.Write(obps::LogLevel::DEBUG, __VA_ARGS__ )
    #else
        #define Debug(...) ;
    #endif // DEBUG_MODE

#else
    #define MakeScopeLog(...) ;
    #define Attach(...) ;
    #define Error(...) ;
    #define Warning(...) ;
    #define Info(...) ;
    #define Debug(...) ;

#endif // LOG_ON