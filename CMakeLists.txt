cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

project(ObpsLog VERSION 0.2)

# Determine ObpsLog build as a standalone project
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

# checking an OS which we build for and setting flags
if (WIN32)
    message("OS: WINDOWS")
    add_definitions(-DWIN32)
elseif (UNIX AND NOT APPLE)
    message("OS: LINUX")
    add_definitions(-DLINUX)
    set(LINUX True)	
endif()

# sourcing configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/Conf.cmake)

# checking build type and setting appropriate flags
if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    message("SET DEBUG_MODE")
    add_definitions(-DDEBUG_MODE)
    add_definitions(-DFLUSH_EVERY_MESSAGE)
elseif (${CMAKE_BUILD_TYPE} STREQUAL Release)
    message("SET RELEASE")
    add_definitions(-DNDEBUG)
endif()


# Logger configuration header
configure_file(ObpsLogConfig.hpp.in ${ObpsLog_SOURCE_DIR}/ObpsLogConfig.hpp)

# define library interface and linking options
add_library(obps_log STATIC obps_log_private.cpp)

set_target_properties( obps_log PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    CXX_STANDARD 20
)

target_include_directories( obps_log
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

if (LINUX)
    target_link_libraries(obps_log PUBLIC pthread)
endif()


################
### TESTING: ###            If main build tests
################

if (MAIN_PROJECT)
    message("Enable Testing:")
    enable_testing(True)

    # Importing gtest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/0134d73a4902574269ff2e42827f7573d3df08ae.zip
        )
    if (WIN32)
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    FetchContent_MakeAvailable(googletest)
    
    
    # simple_test
    #   enabled logging:
    add_executable(test_logging_enabled simple_test.cpp)
    set_target_properties(test_logging_enabled PROPERTIES CXX_STANDARD 20)
    target_compile_options(test_logging_enabled PUBLIC -DLOG_ON)
    target_link_libraries(test_logging_enabled PRIVATE obps_log)
    
    add_test( NAME test_logging_enabled COMMAND test_logging_enabled )

    set_tests_properties(test_logging_enabled PROPERTIES 
        PASS_REGULAR_EXPRESSION "INFO Pass" # expected to log Info Pass into stdout
    ) 
    
    #   disabled logging
    add_executable(test_logging_disabled simple_test.cpp)
    set_target_properties(test_logging_disabled PROPERTIES CXX_STANDARD 20)
    target_link_libraries(test_logging_disabled PRIVATE obps_log)

    add_test( NAME test_logging_disabled COMMAND test_logging_disabled )

    set_tests_properties(test_logging_disabled PROPERTIES 
        PASS_REGULAR_EXPRESSION "^?" # nothing gonna be printed, Logging is off!
    )


endif()