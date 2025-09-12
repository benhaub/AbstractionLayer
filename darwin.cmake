################################################################################
#Date: September 26th, 2023                                                    #
#File: CMakeLists.txt                                                          #
#Authour: Ben Haubrich                                                         #
#Synopsis: Top level project CMakeList.txt for AbstractionLayer desktop build  #
################################################################################
add_subdirectory(${CMAKE_CURRENT_LIST_DIR})

#Modules are chunks of code that implement functionality. They are all interchangeable provided that the target platform
#supports the implementation.
#They serve as a method of porting your main application to different platforms.
#E.g. to run on Linux, replace or add main/Modules/Drivers/Linux/<module> subdirectory.
#To remove modules, use the "None" port since at least a function stub must exist for the main application
#to call.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Uart/Posix)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Watchdog/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Gpio/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/I2c/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Spi/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Rtc/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Pwm/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/PowerResetClockManagement/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Gptm/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Drivers/Adc/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/OperatingSystem/Darwin)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Network/Wifi/Darwin)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Network/Cellular/None)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Http/Posix)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Logging/stdlib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Storage/Darwin)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Error/Errno)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Tools/None/Crc)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/MemoryManagement/Default)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/Processor/None)
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
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IpClient)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Applications/IpServer)