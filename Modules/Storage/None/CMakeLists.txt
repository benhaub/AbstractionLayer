target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  FileSystemModule.hpp
  StorageModule.hpp
)

add_library(NoneStorage
OBJECT
  StorageModule.cpp
  FileSystemModule.cpp
)

target_include_directories(Storage INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneStorage PRIVATE abstractionLayer)
target_link_libraries(NoneStorage PRIVATE Storage)
target_link_libraries(NoneStorage PRIVATE Event)
target_link_libraries(NoneStorage PRIVATE OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneStorage)

target_compile_options(NoneStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Storage,INTERFACE_INCLUDE_DIRECTORIES>)
endif()