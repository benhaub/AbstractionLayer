target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  WifiModule.hpp
)

add_library(LinuxWifi
STATIC
  WifiModule.cpp
)

target_include_directories(Network INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(LinuxWifi PUBLIC abstractionLayer)
target_link_libraries(LinuxWifi PUBLIC Network)
target_link_libraries(LinuxWifi PUBLIC Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC LinuxWifi)

if (ESP_PLATFORM)
  target_include_directories(LinuxWifi PRIVATE $<TARGET_PROPERTY:__idf_main,INTERFACE_INCLUDE_DIRECTORIES>)
  target_include_directories(__idf_main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
else()
  target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
endif()

target_compile_options(LinuxWifi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
