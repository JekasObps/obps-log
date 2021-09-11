#define ObpsLog_VERSION_MAJOR 1
#define ObpsLog_VERSION_MINOR 0

#define DEFAULT_QUEUE_SIZE 64
#define MAX_MSG_SIZE 256

#define OBPS_LOG_LEVELS ERROR, WARN, INFO, USER_LEVEL, DEBUG
#define OBPS_LOG_PRETTY_LEVELS \
    case obps::LogLevel::ERROR: return "ERROR";\
    case obps::LogLevel::WARN: return "WARN";\
    case obps::LogLevel::INFO: return "INFO";\
    case obps::LogLevel::USER_LEVEL: return "USER_LEVEL";\
    case obps::LogLevel::DEBUG: return "DEBUG";\


#ifdef LOG_ON
    #define ERROR(...) _SCOPE_LOG_ID.Write(obps::LogLevel::ERROR, __VA_ARGS__)
    #define WARN(...) _SCOPE_LOG_ID.Write(obps::LogLevel::WARN, __VA_ARGS__)
    #define INFO(...) _SCOPE_LOG_ID.Write(obps::LogLevel::INFO, __VA_ARGS__)
    #define USER_LEVEL(...) _SCOPE_LOG_ID.Write(obps::LogLevel::USER_LEVEL, __VA_ARGS__)
#if defined(DEBUG_MODE) || !defined(NDEBUG)
    #define DEBUG(...) _SCOPE_LOG_ID.Write(obps::LogLevel::DEBUG, __VA_ARGS__)
#else
    #define DEBUG(...)
#endif // DEBUG_MODE
#else
    #define ERROR(...) {}
    #define WARN(...) {}
    #define INFO(...) {}
    #define USER_LEVEL(...) {}
    #define DEBUG(...) {}
#endif //LOG_ON
