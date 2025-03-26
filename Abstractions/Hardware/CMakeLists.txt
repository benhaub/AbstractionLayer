target_sources(abstractionLayer
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  UartAbstraction.hpp
  GpioAbstraction.hpp
  WatchdogAbstraction.hpp
  RtcAbstraction.hpp
  I2cAbstraction.hpp
  PwmAbstraction.hpp
  PowerResetClockManagementAbstraction.hpp
  GptmAbstraction.hpp
  GptmPwmAbstraction.hpp
  GptmCaptureAbstraction.hpp
  GptmTimerAbstraction.hpp
  SpiAbstraction.hpp
  AdcAbstraction.hpp
  HBridgeAbstraction.hpp
  DarlingtonArrayAbstraction.hpp
)

add_library(Uart INTERFACE)
add_library(Gpio INTERFACE)
add_library(Watchdog INTERFACE)
add_library(Rtc INTERFACE)
add_library(I2c INTERFACE)
add_library(Spi INTERFACE)
add_library(Pwm INTERFACE)
add_library(PowerResetClockManagement INTERFACE)
add_library(Gptm INTERFACE)
add_library(Adc INTERFACE)
add_library(HBridge INTERFACE)
add_library(DarlingtonArray INTERFACE)

target_include_directories(Uart INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Gpio INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Watchdog INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Rtc INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(I2c INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Spi INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Pwm INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(PowerResetClockManagement INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Gptm INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Adc INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(HBridge INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(DarlingtonArray INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})