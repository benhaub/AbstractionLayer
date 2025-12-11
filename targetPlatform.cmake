################################################################################
#Date: September 26th, 2023                                                    #
#File: CMakeLists.txt                                                          #
#Authour: Ben Haubrich                                                         #
#Synopsis: Top level project CMakeList.txt for AbstractionLayer cc32xx build   #
################################################################################
add_subdirectory(${CMAKE_CURRENT_LIST_DIR})

#Modules are chunks of code that implement functionality. They are all interchangeable provided that the target platform
#supports the implementation.
#They serve as a method of porting your main application to different platforms.
#E.g. to run on Linux, replace or add main/Modules/Drivers/Linux/<module> subdirectory.
#To remove modules, use the "None" port since at least a function stub must exist for the main application
#to call.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Uart/${UART_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Watchdog/${WATCHDOG_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Gpio/${GPIO_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/I2c/${I2C_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Spi/${SPI_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Rtc/${RTC_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Pwm/${PWM_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/PowerResetClockManagement/${POWER_RESET_CLOCK_MANAGEMENT_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Gptm/${GPTM_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Adc/${ADC_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/OperatingSystem/${OPERATING_SYSTEM_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Network/Wifi/${WIFI_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Network/Cellular/${CELLULAR_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Http/${HTTP_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Logging/${LOGGING_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Storage/${STORAGE_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Error/${ERROR_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Tools/${CRC_MODULE_TYPE}/Crc)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/MemoryManagement/${MEMORY_MANAGEMENT_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Processor/${PROCESSOR_MODULE_TYPE})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Utilities)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Logging)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Event)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/MbedTlsCompatibility)
if (ENABLE_COMMAND_QUEUE)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/CommandQueue)
endif()
if (ENABLE_SIGNALS_AND_SLOTS)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/SignalsAndSlots)
endif()
if (ENABLE_FACTORIES)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Factories)
endif()
if (ENABLE_MEMORY_POOL)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/MemoryPool)
endif()
if (ENABLE_SM10001)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Peripherals/Adafruit/Sm10001)
endif()
if (ENABLE_28BYJ485V)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Peripherals/Mikroe/28byj485V)
endif()
if (ENABLE_HBRIDGE)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IntegratedCircuits/HBridge)
endif()
if (ENABLE_DARLINGTON_ARRAY)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IntegratedCircuits/DarlingtonArray)
endif()
if (ENABLE_EXTERNAL_RTC)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IntegratedCircuits/Rtc)
endif()
if (ENABLE_RTC_MANAGER)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/RtcManager)
endif()
if (ENABLE_NTP_CLIENT)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/NtpClient)
endif()
if (ENABLE_TICK_TIMER)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/TickTimer)
endif()
if (ENABLE_IP_CLIENT)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IpClient)
endif()
if (ENABLE_IP_SERVER)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IpServer)
endif()

if (TARGET_PLATFORM STREQUAL "Cc32xx")
  target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    SIMPLELINK_THREAD_NAME="simplelinkTask" 
  )

  target_compile_options(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fconcepts>
  )
elseif(TARGET_PLATFORM STREQUAL "Tm4c123")
  target_compile_options(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
  PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fconcepts>
  )
elseif(TARGET_PLATFORM STREQUAL "Esp" OR TARGET_PLATFORM STREQUAL "Linux" OR TARGET_PLATFORM STREQUAL "Darwin" OR TARGET_PLATFORM STREQUAL "Raspbian12Pi4ModelB")
  #All other implemented platforms that are known.
else()
  message(FATAL_ERROR "Unknown TARGET_PLATFORM: ${TARGET_PLATFORM}. See Modules for available platforms.")
endif()