target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  OperatingSystemModule.hpp
)

add_library(LinuxOperatingSystem
OBJECT
  OperatingSystemModule.cpp
)

target_include_directories(OperatingSystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(LinuxOperatingSystem PRIVATE abstractionLayer)
target_link_libraries(LinuxOperatingSystem PRIVATE OperatingSystem)
target_link_libraries(LinuxOperatingSystem PRIVATE MemoryPool)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE LinuxOperatingSystem)

target_include_directories(LinuxOperatingSystem PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(LinuxOperatingSystem PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(LinuxOperatingSystem PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)