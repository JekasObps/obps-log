cmake_minimum_required(VERSION 3.20)

project(ObpsLog VERSION 0.2)

# Determine ObpsLog build as a standalone project
set(MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

# setting up gtest
set(CMAKE_CXX_STANDARD 20)
  
if (WIN32)
    message("OS: WINDOWS")
    add_definitions(-DWIN32)

elseif(UNIX AND NOT APPLE)
    message("OS: LINUX")
    add_definitions(-DLINUX)
    set(LINUX True)	
endif()

# sourcing configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/Conf.cmake)

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    message("SET DEBUG_MODE")
    add_definitions(-DDEBUG_MODE)
    add_definitions(-DFLUSH_EVERY_MESSAGE)
elseif (${CMAKE_BUILD_TYPE} STREQUAL Release)
    message("SET RELEASE")
    add_definitions(-DNDEBUG)
endif()

add_library(obps_log STATIC obps_log.cpp)

set_target_properties( obps_log PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib
    CXX_STANDARD 20
)

target_include_directories( obps_log
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

if (LINUX)
    target_link_libraries(obps_log PUBLIC pthread)
endif()

function(EnableLogging enable)
    if (${enable})
        message("ENABLING LOGGING")
        set(LOG_ON True)
    else()
        unset(LOG_ON)
    endif()
    configure_file(ObpsLogConfig.hpp.in ${ObpsLog_SOURCE_DIR}/ObpsLogConfig.hpp)
endfunction()

EnableLogging(ENABLE_LOGGING)
    
### TESTING:

if (MAIN_PROJECT)
    message("Enable Testing:")
    enable_testing(True)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/0134d73a4902574269ff2e42827f7573d3df08ae.zip
        )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    add_executable(log_test test.cpp)
    # add_executable(log_test log_test.cpp)

    if (ENABLE_LOGGING)
        target_link_libraries(log_test PUBLIC obps_log)
    endif()

    set_target_properties(log_test PROPERTIES CXX_STANDARD 20)
    add_test( NAME log_test COMMAND log_test )

endif()