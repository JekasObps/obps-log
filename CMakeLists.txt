cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

project(ObpsLog VERSION 1.0)

include(Import.cmake)
import_build_scripts()


# Determine ObpsLog build as a standalone project
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

#TODO: move to build tools
# checking an OS which we build for and setting flags
if (WIN32)
    message(STATUS "OS: WINDOWS")
    add_definitions(-DWIN32)
elseif (UNIX AND NOT APPLE)
    message(STATUS "OS: LINUX")
    add_definitions(-DLINUX)
    set(LINUX True)
endif()

#TODO: move to build tools
# checking build type and setting appropriate flags
if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    message(STATUS "SET DEBUG_MODE")
    add_definitions(-DDEBUG_MODE)
elseif (${CMAKE_BUILD_TYPE} STREQUAL Release)
    message(STATUS "SET RELEASE")
    add_definitions(-DNDEBUG)
endif()

###
#   CONFIG
###

# sourcing configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/Conf.cmake)

# setting up log levels
include(${CMAKE_CURRENT_SOURCE_DIR}/SetLogLevels.cmake)

# Logger configuration header
configure_file(ObpsLogConfig.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/ObpsLogConfig.hpp)


import_repo(data_structures https://github.com/JekasObps/obps-data-structures.git main)

enable_testing(on)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)