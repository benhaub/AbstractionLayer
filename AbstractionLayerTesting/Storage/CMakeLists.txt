add_executable(StorageTest
  StorageTest.cpp
)

target_include_directories(StorageTest
PRIVATE
  ${CMAKE_SOURCE_DIR}/../Abstractions/Storage
  ${CMAKE_SOURCE_DIR}/../Abstractions/OperatingSystem
  ${CMAKE_SOURCE_DIR}/../Abstractions/Logging

  ${CMAKE_SOURCE_DIR}/../Modules/Error/Errno
  ${CMAKE_SOURCE_DIR}/../Modules/Storage/${CMAKE_HOST_SYSTEM_NAME}
  ${CMAKE_SOURCE_DIR}/../Modules/Logging/stdlib
  ${CMAKE_SOURCE_DIR}/../Modules/OperatingSystem/${CMAKE_HOST_SYSTEM_NAME}

  ${CMAKE_SOURCE_DIR}/../Utilities

  ${CMAKE_SOURCE_DIR}/../Applications/Event
  ${CMAKE_SOURCE_DIR}/../Applications/Logging
)

find_library(errorLib
NAMES
  ErrnoError
HINTS
  ${buildDir}/AbstractionLayer/Modules/Error/Errno
)

find_library(loggerLib
NAMES
  StdlibLogger
HINTS
  ${buildDir}/AbstractionLayer/Modules/Logging/stdlib
)

find_library(operatingSystemLib
NAMES
  ${CMAKE_HOST_SYSTEM_NAME}OperatingSystem
HINTS
  ${buildDir}/AbstractionLayer/Modules/OperatingSystem/${CMAKE_HOST_SYSTEM_NAME}
)

find_library(eventLib
NAMES
  Event
HINTS
  ${buildDir}/AbstractionLayer/Applications/Event
)

find_library(storageLib
NAMES
  ${CMAKE_HOST_SYSTEM_NAME}Storage
HINTS
  ${buildDir}/AbstractionLayer/Modules/Storage/${CMAKE_HOST_SYSTEM_NAME}
)

target_compile_options(StorageTest PRIVATE $<TARGET_PROPERTY:abstractionLayerTesting,INTERFACE_COMPILE_OPTIONS>)

target_link_libraries(StorageTest PRIVATE ${storageLib})
target_link_libraries(StorageTest PRIVATE ${operatingSystemLib})
target_link_libraries(StorageTest PRIVATE ${errorLib})
target_link_libraries(StorageTest PRIVATE ${loggerLib})
target_link_libraries(StorageTest PRIVATE ${eventLib})


add_test(
  NAME Storage
  COMMAND StorageTest
)

set_property(TEST Storage
PROPERTY
  TIMEOUT 10
)