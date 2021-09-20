# setup log levels
set(__LOG_LEVELS "${OBPS_LOG_LEVELS}")
string(REPLACE ";" ", " OBPS_LOG_LEVELS "${OBPS_LOG_LEVELS}")
message("log levels: ${OBPS_LOG_LEVELS}")

list(APPEND OBPS_LOG_PRETTY_LEVELS "\\\n")
foreach(level ${__LOG_LEVELS})
    list(APPEND OBPS_LOG_PRETTY_LEVELS
        "    case obps::LogLevel::${level}: return \"${level}\";\\\n"
    )

    if (level STREQUAL "DEBUG")
        list(APPEND OBPS_LOG_MACROS__ "#if defined(DEBUG_MODE) || !defined(NDEBUG)")
    endif()

    list(APPEND OBPS_LOG_MACROS__ 
        "    #define ${level}(...) _SCOPE_LOG_ID.Write(obps::LogLevel::${level}, false, __VA_ARGS__)"
        "    #define G_${level}(...) get_global_log().Write(obps::LogLevel::${level}, false, __VA_ARGS__)"
        "    #define ${level}_SYNC(...) _SCOPE_LOG_ID.Write(obps::LogLevel::${level}, true, __VA_ARGS__)"
        "    #define G_${level}_SYNC(...) get_global_log().Write(obps::LogLevel::${level}, true, __VA_ARGS__)"
        "    #define MUTE(...) _SCOPE_LOG_ID.Mute(__VA_ARGS__)"
        "    #define G_MUTE(...) get_global_log().Mute(__VA_ARGS__)"
        "    #define UNMUTE(...) _SCOPE_LOG_ID.Unmute(__VA_ARGS__)"
        "    #define G_UNMUTE(...) get_global_log().Unmute(__VA_ARGS__)"
    )

    if (level STREQUAL "DEBUG")
        list(APPEND OBPS_LOG_MACROS__ 
            "#else" 
            "    #define DEBUG(...) {}" 
            "    #define G_DEBUG(...) {}" 
            "#endif // DEBUG_MODE"
        )
    endif()
        
    list(APPEND OBPS_LOG_MACROS_OFF__ 
        "    #define ${level}(...) {}"
        "    #define G_${level}(...) {}"
        "    #define ${level}_SYNC(...) {}"
        "    #define G_${level}_SYNC(...) {}"
        "    #define MUTE(...) {}"
        "    #define G_MUTE(...) {}"
        "    #define UNMUTE(...) {}"
        "    #define G_UNMUTE(...) {}"
    )
endforeach()

string(REPLACE ";" "\n" OBPS_LOG_MACROS__ "${OBPS_LOG_MACROS__}")
string(REPLACE ";" "\n" OBPS_LOG_MACROS_OFF__ "${OBPS_LOG_MACROS_OFF__}")
string(REPLACE "; " " " OBPS_LOG_PRETTY_LEVELS "${OBPS_LOG_PRETTY_LEVELS}")