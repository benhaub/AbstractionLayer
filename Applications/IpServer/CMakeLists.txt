target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpServer.hpp
)

add_library(IpServer
OBJECT
  IpServer.cpp
)

target_include_directories(IpServer PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(IpServer PRIVATE abstractionLayer)
target_link_libraries(IpServer PRIVATE Logging)
target_link_libraries(IpServer PRIVATE OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE IpServer)

target_compile_options(IpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(IpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()