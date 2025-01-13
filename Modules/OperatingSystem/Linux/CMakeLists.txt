target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  OperatingSystemModule.hpp
)

add_library(LinuxOperatingSystem
STATIC
  OperatingSystemModule.cpp
)

target_include_directories(OperatingSystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(LinuxOperatingSystem PUBLIC abstractionLayer)
target_link_libraries(LinuxOperatingSystem PUBLIC OperatingSystem)
target_link_libraries(LinuxOperatingSystem PUBLIC Utilities)
target_link_libraries(LinuxOperatingSystem PUBLIC Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC LinuxOperatingSystem)

target_include_directories(LinuxOperatingSystem PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(LinuxOperatingSystem PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
