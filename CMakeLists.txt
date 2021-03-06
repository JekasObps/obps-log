cmake_minimum_required(VERSION 3.8)

project(ObpsLog VERSION 0.2)

configure_file(ObpsLogConfig.hpp.in ${ObpsLog_SOURCE_DIR}/ObpsLogConfig.hpp)

if (WIN32)
    add_definitions(-DWIN32)
elseif(UNIX and NOT APPLE)
    add_definitions(-DLINUX)
endif()

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

###############################       TEST        ##############################

enable_testing(True)

add_executable(log_test log_test.cpp)
target_link_libraries(log_test PUBLIC obps_log)

add_test( NAME log_test COMMAND log_test )
