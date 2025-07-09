add_executable(ExampleTest
  ExampleTest.cpp
)

target_include_directories(ExampleTest
PRIVATE
  ${CMAKE_SOURCE_DIR}/../Abstractions/Logging
  ${CMAKE_SOURCE_DIR}/../Abstractions/OperatingSystem
  ${CMAKE_SOURCE_DIR}/../Utilities
  ${CMAKE_SOURCE_DIR}/../Modules/Error/Errno
  ${CMAKE_SOURCE_DIR}/../Modules/Logging/stdlib
  ${CMAKE_SOURCE_DIR}/../Modules/OperatingSystem/${CMAKE_HOST_SYSTEM_NAME}
  ${CMAKE_SOURCE_DIR}/../Applications/Logging
  ${CMAKE_SOURCE_DIR}/../Applications/Event
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

target_compile_options(ExampleTest PRIVATE $<TARGET_PROPERTY:abstractionLayerTesting,INTERFACE_COMPILE_OPTIONS>)

target_link_libraries(ExampleTest PRIVATE ${errorLib})
target_link_libraries(ExampleTest PRIVATE ${loggerLib})
target_link_libraries(ExampleTest PRIVATE ${operatingSystemLib})
target_link_libraries(ExampleTest PRIVATE ${eventLib})

add_test(
  NAME Example
  COMMAND ExampleTest
)

set_property(TEST Example
PROPERTY
  TIMEOUT 10
)