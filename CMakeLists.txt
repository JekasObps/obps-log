cmake_minimum_required(VERSION 3.8)

project(ObpsLog VERSION 0.2)

# setting up gtest
set(CMAKE_CXX_STANDARD 11)
  
if (WIN32)
    message("OS: WINDOWS")
    add_definitions(-DWIN32)
   
    execute_process(
        COMMAND powershell "Remove-item -recurse -force ${CMAKE_CURRENT_SOURCE_DIR}/build/_deps"
        RESULT_VARIABLE result
        )
    message("Removing ${CMAKE_CURRENT_SOURCE_DIR}/build/_deps:\r\n" ${result})   

elseif(UNIX AND NOT APPLE)
    message("OS: LINUX")
    add_definitions(-DLINUX)
    set(LINUX True)	
endif()


include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/0134d73a4902574269ff2e42827f7573d3df08ae.zip
    )
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# sourcing configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/Conf.cmake)
configure_file(ObpsLogConfig.hpp.in ${ObpsLog_SOURCE_DIR}/ObpsLogConfig.hpp)

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    message("SET DEBUG_MODE")
    add_definitions(-DDEBUG_MODE)
    add_definitions(-DFLUSH_EVERY_MESSAGE)
elseif (${CMAKE_BUILD_TYPE} STREQUAL Release)
    message("SET RELEASE")
    add_definitions(-DNDEBUG)
endif()

if (ENABLE_LOGGING)
    message("ENABLING LOGGING")
    add_definitions(-DLOG_ON)
    add_library(obps_log STATIC obps_log.cpp)
    
    set_target_properties( obps_log PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib
        CXX_STANDARD 20
        )

    target_include_directories( obps_log
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        )

    if(LINUX)
        target_link_libraries(obps_log PUBLIC pthread)
    endif()

else()
    remove_definitions(-DLOG_ON)
endif() # ENABLE_LOGGING
###############################       TEST        ##############################

enable_testing(True)

add_executable(log_test test.cpp)
# add_executable(log_test log_test.cpp)

if (ENABLE_LOGGING)
    target_link_libraries(log_test PUBLIC obps_log)
endif()

set_target_properties(log_test PROPERTIES CXX_STANDARD 11)
add_test( NAME log_test COMMAND log_test )