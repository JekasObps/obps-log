#pragma once

#include "ObpsLogConfig.hpp"

#ifdef LOG_ON
    #include "obps_log_private.hpp"
    #include "log_scope_destroyer.hpp"

    #define CONCAT(a, b) a ## b
    #define EXP(line, suf) CONCAT(line, suf)

    #define _GLOBAL_LOG_ID __obps_global_log
    #define _GLOBAL_LOG_INIT_FUNC __obps_global_log_init
    #define _SCOPE_LOG_ID __obps_scope_log
    #define _LOG_DESTROYER_ID __obps_log_destroyer

    /*
    *   Register propper log shutdown. 
    *   User must place this macro at the beginning of the main function.
    *   or another scope that will limit log opperation.
    */
    #define OBPS_LOG_INIT() \
        obps::LogScopeDestroyer _LOG_DESTROYER_ID
    
    /*
    *   Create static log in global scope.  TODO:  
    */
    #define GLOBAL_LOG(level, file_name_or_stream, ...) \
        // static std::shared_ptr<obps::Log> _GLOBAL_LOG_ID = std::make_shared<obps::Log>(\
        //     obps::LogBase::LogSpecs{file_name_or_stream, obps::LogLevel::level, __VA_ARGS__})

    /*
    *   Call at the beginning of the logging scope
    */
    #define SCOPE_LOG(...) \
        static auto _SCOPE_LOG_ID = obps::Log({__VA_ARGS__})

#else
    #define OBPS_LOG_INIT() ;
    #define GLOBAL_LOG(...) ; 
    #define SCOPE_LOG(...) ;
#endif // LOG_ON