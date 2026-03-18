target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  FileSystemModule.hpp
  StorageModule.hpp
)

add_library(LinuxStorage
OBJECT
  StorageModule.cpp
  FileSystemModule.cpp
)

target_include_directories(Storage INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(LinuxStorage PRIVATE abstractionLayer)
target_link_libraries(LinuxStorage PRIVATE Storage)
target_link_libraries(LinuxStorage PRIVATE OperatingSystem)
target_link_libraries(LinuxStorage PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE LinuxStorage)

target_compile_options(LinuxStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(LinuxStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)