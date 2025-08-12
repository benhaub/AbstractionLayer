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
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Uart/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Watchdog/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Gpio/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/I2c/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Spi/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Rtc/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Pwm/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/PowerResetClockManagement/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Gptm/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Adc/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/OperatingSystem/Cc32xx/FreeRTOS)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Network/Wifi/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Network/Cellular/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Ip/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Logging/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Storage/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Error/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Tools/None/Crc)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/MemoryManagement/Cc32xx/FreeRTOS)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Processor/Cc32xx)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Utilities)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Logging)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/CommandQueue)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Event)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/SignalsAndSlots)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Factories)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/MemoryPool)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Peripherals/Adafruit/Sm10001)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/Peripherals/Mikroe/28byj485V)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IntegratedCircuits/HBridge)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IntegratedCircuits/DarlingtonArray)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IntegratedCircuits/Rtc)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/RtcManager)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/NtpClient)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/TickTimer)

target_compile_definitions(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE
  SIMPLELINK_THREAD_NAME="simplelinkTask" 
)

target_compile_options(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-fconcepts>
)
