# setup log levels
set(__LOG_LEVELS "${OBPS_LOG_LEVELS}")
string(REPLACE ";" "," OBPS_LOG_LEVELS "${OBPS_LOG_LEVELS}")
message("log levels: ${OBPS_LOG_LEVELS}")

foreach(level ${__LOG_LEVELS})
    list(APPEND OBPS_LOG_PRETTY_LEVELS 
        "case obps::LogLevel::${level}: return \"${level}\""
    )

    if (level STREQUAL "DEBUG")
        list(APPEND OBPS_LOG_MACROS__ "#if defined(DEBUG_MODE) || !defined(NDEBUG)")
    endif()

    list(APPEND OBPS_LOG_MACROS__ 
        "#define ${level}(...) _SCOPE_LOG_ID.Write(obps::LogLevel::${level}, __VA_ARGS__)"
    )

    if (level STREQUAL "DEBUG")
        list(APPEND OBPS_LOG_MACROS__ "#else" "#define DEBUG(...)" "#endif // DEBUG_MODE")
    endif()
        
    list(APPEND OBPS_LOG_MACROS_OFF__ "#define ${level}(...);" )
endforeach()

string(REPLACE ";" "\n" OBPS_LOG_MACROS__ "${OBPS_LOG_MACROS__}")
string(REPLACE ";" "\n" OBPS_LOG_MACROS_OFF__ "${OBPS_LOG_MACROS_OFF__}")