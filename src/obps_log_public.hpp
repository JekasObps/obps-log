#pragma once

#include "ObpsLogConfig.hpp"

#ifdef LOG_ON
    #include "obps_log_private.hpp"
    
    // exposing LogLevel to a user namespace
    namespace
    {
        using LogLevel = obps::LogLevel;
    }

    #define CONCAT(a, b) a ## b
    #define EXP(line, suf) CONCAT(line, suf)

    #define _GLOBAL_LOG_ID __obps_global_log
    #define _GLOBAL_LOG_INIT_FUNC __obps_global_log_init
    #define _SCOPE_LOG_ID __obps_scope_log

    /*
    *   Register propper log shutdown. 
    *   User must place this macro at the beginning of the main function.
    *   or another scope that will limit log opperation.
    */
    #define OBPS_LOG_TEARDOWN() obps::LogRegistry::ObpsLogShutdown()
    
    /*
    *   Create log in global scope.
    */
    #define GLOBAL_LOG(...) \
        obps::Log _GLOBAL_LOG_ID = obps::Log({__VA_ARGS__}); \
        obps::Log& get_global_log(){ return _GLOBAL_LOG_ID; } \
        // getter helps to ensure that a global log has been initialized on cross translation unit access 

    #define G_MUTE(...) get_global_log().Mute({__VA_ARGS__})
    #define G_UNMUTE(...) get_global_log().Unmute({__VA_ARGS__})

    /*
    *   Call at the beginning of the logging scope
    */
    #define SCOPE_LOG(...) \
        static obps::Log _SCOPE_LOG_ID({__VA_ARGS__})

    #define MUTE(...) _SCOPE_LOG_ID.Mute({__VA_ARGS__})
    #define UNMUTE(...) _SCOPE_LOG_ID.Unmute({__VA_ARGS__})
#else
    #define OBPS_LOG_TEARDOWN() {}
    #define GLOBAL_LOG(...)
    #define SCOPE_LOG(...)

    #define MUTE(...) {}
    #define G_MUTE(...) {}
    #define UNMUTE(...) {}
    #define G_UNMUTE(...) {}

#endif // LOG_ON