target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  FileSystemModule.hpp
  StorageModule.hpp
)

add_library(DarwinStorage
STATIC
  StorageModule.cpp
  FileSystemModule.cpp
)

target_include_directories(Storage INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(DarwinStorage PUBLIC abstractionLayer)
target_link_libraries(DarwinStorage PUBLIC Storage)
target_link_libraries(DarwinStorage PUBLIC Logging)
target_link_libraries(DarwinStorage PUBLIC OperatingSystem)
target_link_libraries(DarwinStorage PUBLIC Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC DarwinStorage)

target_compile_options(DarwinStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(DarwinStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)