target_sources(abstractionLayer
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  UartAbstraction.hpp
  GpioAbstraction.hpp
  WatchdogAbstraction.hpp
  RtcAbstraction.hpp
  I2cAbstraction.hpp
  PowerAndClockManagementAbstraction.hpp
)

add_library(Uart INTERFACE)
add_library(Gpio INTERFACE)
add_library(Watchdog INTERFACE)
add_library(Rtc INTERFACE)
add_library(I2c INTERFACE)
add_library(PowerAndClock INTERFACE)

target_include_directories(Uart INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Gpio INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Watchdog INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Rtc INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(I2c INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(PowerAndClock INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
