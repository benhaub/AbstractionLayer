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
target_link_libraries(LinuxMpsseSpi PRIVATE Spi)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE LinuxMpsseSpi)

target_include_directories(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

set(LibMPSSE "libmpsse-x86_64-1.0.8.tgz")
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_LIST_DIR}/${LibMPSSE}
    COMMAND
      rm -rf ${LibMPSSE}
    COMMAND
      wget https://ftdichip.com/wp-content/uploads/2025/08/libmpsse-x86_64-1.0.8.tgz
    COMMAND
      tar -xzf ${LibMPSSE}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_LIST_DIR}
    COMMENT
      "Downloading ${LibMPSSE} for Linux"
    VERBATIM
)
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_LIST_DIR}/release/build/native/libmpsse.a
    COMMAND
      bash -c "mkdir -p build/native/source build/native/include && make"
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_LIST_DIR}/release
    COMMENT
      "Building ${LibMPSSE} for Linux"
    VERBATIM
)
add_custom_target(libmpsse_download DEPENDS ${LibMPSSE})
add_custom_target(libmpsse_build DEPENDS ${CMAKE_CURRENT_LIST_DIR}/release/build/native/libmpsse.a)
add_dependencies(libmpsse_build libmpsse_download)
add_dependencies(LinuxMpsseSpi libmpsse_build)

target_link_libraries(LinuxMpsseSpi PRIVATE ${CMAKE_CURRENT_LIST_DIR}/release/build/native/libmpsse.a)

target_include_directories(LinuxMpsseSpi PUBLIC ${CMAKE_CURRENT_LIST_DIR}/release/include)
target_include_directories(LinuxMpsseSpi PUBLIC ${CMAKE_CURRENT_LIST_DIR}/release/libftd2xx)

target_compile_options(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(LinuxMpsseSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)