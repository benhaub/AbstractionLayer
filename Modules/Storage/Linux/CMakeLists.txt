target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  FileSystemModule.hpp
  StorageModule.hpp
)

add_library(LinuxStorage
STATIC
  StorageModule.cpp
  FileSystemModule.cpp
)

target_include_directories(Storage INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(LinuxStorage PUBLIC abstractionLayer)
target_link_libraries(LinuxStorage PUBLIC Storage)
target_link_libraries(LinuxStorage PUBLIC Logging)
target_link_libraries(LinuxStorage PUBLIC OperatingSystem)
target_link_libraries(LinuxStorage PUBLIC Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC LinuxStorage)

target_compile_options(LinuxStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(LinuxStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)