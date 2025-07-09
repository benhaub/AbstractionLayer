################################################################################
#Date: May 9th, 2024                                                           #
#File: CMakeLists.txt                                                          #
#Authour: Ben Haubrich                                                         #
#Synopsis: Top level project CMakeList.txt for testing the abstraction layer   #
################################################################################
cmake_minimum_required(VERSION 3.5)

project(abstractionLayerTesting)

include(CTest)

set(buildDir "${CMAKE_CURRENT_LIST_DIR}/../../${CMAKE_HOST_SYSTEM_NAME}_build")

if(NOT EXISTS ${buildDir}/AbstractionLayer)
  message(FATAL_ERROR "Could not find AbstractionLayer build files in set build directory: ${buildDir}")
endif()

add_library(abstractionLayerTesting INTERFACE)

target_compile_options(abstractionLayerTesting
INTERFACE
  -ffunction-sections
  -fdata-sections
  -fno-exceptions
  -O0
  -g3
  -ggdb
  -Wextra
  -std=gnu++23
)

add_subdirectory(Example)
add_subdirectory(OperatingSystem)
add_subdirectory(Storage)
add_subdirectory(Ip)