cmake_minimum_required(VERSION 3.5)

project(ObpsLog VERSION 0.2)

configure_file(ObpsLogConfig.hpp.in ObpsLogConfig.hpp)

add_definitions(-DLOG_ON)

add_library(obps_log STATIC obps_log.cpp)

set_target_properties( obps_log PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib
    )

target_include_directories( obps_log
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(obps_log
	PUBLIC pthread)

###############################       TEST        ##############################

enable_testing(True)

# add_executable(test_circular_queue test_circular_queue.cpp)
# target_link_libraries(test_circular_queue PUBLIC pthread)
# add_test(NAME test_circular_queue
#          COMMAND test_circular_queue )

# add_executable(log_queue_test log_queue_test.cpp)
add_executable(log_test log_test.cpp)
target_link_libraries(log_test PUBLIC obps_log)

# target_link_libraries(test1 PUBLIC obps_log pthread)
# add_test(NAME log_queue_test
#         COMMAND log_queue_test )

add_test(NAME log_test
        COMMAND log_test )
