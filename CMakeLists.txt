cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

project(obps_log VERSION 1.0)

include(Import.cmake)
import_build_scripts()

check_main_project()
check_build()

###
#   CONFIGURATION
###

# sourcing configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/Conf.cmake)

# setting up log levels
include(${CMAKE_CURRENT_SOURCE_DIR}/SetLogLevels.cmake)

# Logger configuration header
configure_file(ObpsLogConfig.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/ObpsLogConfig.hpp)


enable_subproject_tests(abstract_factory None) # disables abstract_factory tests
#        must be called before importing data_structures in order to take effect
import_repo(data_structures https://github.com/JekasObps/obps-data-structures.git async_thread_pool) 

import_gtest()

enable_tests( TEST_ALL ) # enabling project local tests

enable_testing(on)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/tests)