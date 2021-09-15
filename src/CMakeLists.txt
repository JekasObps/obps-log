# define library interface and linking options
add_library(obps_log STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/log_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/obps_log_private.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log_registry.cpp
)


target_include_directories(obps_log
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

if (LINUX)
    target_link_libraries(obps_log PRIVATE pthread)
endif()

target_link_libraries(obps_log PUBLIC thread_pool ring_queue)