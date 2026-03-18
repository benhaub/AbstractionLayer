target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClient.hpp
)

add_library(IpClient
OBJECT
  IpClient.cpp
)

target_include_directories(IpClient PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(IpClient PRIVATE abstractionLayer)
target_link_libraries(IpClient PRIVATE Logging)
target_link_libraries(IpClient PUBLIC OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE IpClient)

target_compile_options(IpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(IpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()