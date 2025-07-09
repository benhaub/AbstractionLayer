add_executable(OperatingSystemTest
  OperatingSystemTest.cpp
)

target_include_directories(OperatingSystemTest
PRIVATE
  ${CMAKE_SOURCE_DIR}/../Abstractions/OperatingSystem
  ${CMAKE_SOURCE_DIR}/../Abstractions/Logging

  ${CMAKE_SOURCE_DIR}/../Modules/Error/Errno
  ${CMAKE_SOURCE_DIR}/../Modules/Logging/stdlib
  ${CMAKE_SOURCE_DIR}/../Modules/OperatingSystem/${CMAKE_HOST_SYSTEM_NAME}

  ${CMAKE_SOURCE_DIR}/../Applications/Logging
  ${CMAKE_SOURCE_DIR}/../Applications/Event

  ${CMAKE_SOURCE_DIR}/../Utilities
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

target_compile_options(OperatingSystemTest PRIVATE $<TARGET_PROPERTY:abstractionLayerTesting,INTERFACE_COMPILE_OPTIONS>)

target_link_libraries(OperatingSystemTest PRIVATE ${errorLib})
target_link_libraries(OperatingSystemTest PRIVATE ${loggerLib})
target_link_libraries(OperatingSystemTest PRIVATE ${operatingSystemLib})
target_link_libraries(OperatingSystemTest PRIVATE ${eventLib})

target_link_libraries(OperatingSystemTest PRIVATE ${operatingSystemLib})

add_test(
  NAME OperatingSystem
  COMMAND OperatingSystemTest
)

set_property(TEST OperatingSystem
PROPERTY
  TIMEOUT 10
)