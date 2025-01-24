target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  StorageModule.hpp
  FileModule.hpp
)

add_library(NoneStorage
STATIC
  StorageModule.cpp
  FileModule.cpp
)

target_include_directories(Storage INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneStorage PUBLIC abstractionLayer)
target_link_libraries(NoneStorage PUBLIC Storage)
target_link_libraries(NoneStorage PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneStorage)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Storage,INTERFACE_INCLUDE_DIRECTORIES>)
endif()