target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  SpiModule.hpp
)

add_library(LinuxMpsseSpi
OBJECT
  SpiModule.cpp
)

target_include_directories(Spi INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(LinuxMpsseSpi PRIVATE abstractionLayer)
target_link_libraries(LinuxMpsseSpi PRIVATE Logging)
target_link_libraries(LinuxMpsseSpi PRIVATE Spi)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE LinuxMpsseSpi)

target_include_directories(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

#Downloaded from: https://ftdichip.com/software-examples/mpsse-projects/libmpsse-spi-examples/
set(LibMPSSE "libmpsse-x86_64-1.0.8.tgz")
#Downloaded from: https://ftdichip.com/drivers/d2xx-drivers/
set(LibFtd2xx "libftd2xx-linux-x86_64-1.4.34.tgz")
set(LibMPSSEExtractOutput "${CMAKE_CURRENT_LIST_DIR}/release")
set(LibMPSSEBuildOutput "${CMAKE_CURRENT_LIST_DIR}/release/build/native/libmpsse.a")
set(LibFtd2xxOutput "${CMAKE_CURRENT_LIST_DIR}/linux-x86_64/libftd2xx.so")
add_custom_command(
  OUTPUT
    ${LibMPSSEExtractOutput}
  COMMAND
    tar -xzf ${LibMPSSE}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}
  COMMENT
    "Extracting ${LibMPSSE} for Linux"
  VERBATIM
)
add_custom_command(
  OUTPUT
    ${LibMPSSEBuildOutput}
  COMMAND
    bash -c "mkdir -p build/native/source build/native/include && make"
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}/release
  COMMENT
    "Building ${LibMPSSE} for Linux"
  VERBATIM
)
add_custom_command(
  OUTPUT
    ${LibFtd2xxOutput}
  COMMAND
    tar -xzf ${LibFtd2xx}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}
  COMMENT
    "Extracting ${LibFtd2xx} for Linux"
  VERBATIM
)
#As long as the custom target depends on the output of a custom command, and the target is a
#dependancy of another target being build, the custom command will run.
add_custom_target(libmpsse_extract DEPENDS ${LibMPSSEExtractOutput})
add_custom_target(libmpsse_build DEPENDS ${LibMPSSEBuildOutput})
add_custom_target(libftd2xx_extract DEPENDS ${LibFtd2xxOutput})
add_dependencies(libmpsse_build libmpsse_extract)
add_dependencies(LinuxMpsseSpi libmpsse_build)
add_dependencies(LinuxMpsseSpi libftd2xx_extract)

set_target_properties(LinuxMpsseSpi
PROPERTIES
  BUILD_WITH_INSTALL_RPATH TRUE
  INSTALL_RPATH ${CMAKE_CURRENT_LIST_DIR}
  LINK_FLAGS "-Wl,--enable-new-dtags"
)

target_link_libraries(LinuxMpsseSpi
PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/release/build/native/libmpsse.a
  ${CMAKE_CURRENT_LIST_DIR}/linux-x86_64/libftd2xx.so
)

target_include_directories(LinuxMpsseSpi PUBLIC ${CMAKE_CURRENT_LIST_DIR}/release/include)
target_include_directories(LinuxMpsseSpi PUBLIC ${CMAKE_CURRENT_LIST_DIR}/release/libftd2xx)

target_compile_options(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)