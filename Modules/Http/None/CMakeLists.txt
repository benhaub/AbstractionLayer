target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  HttpServerModule.hpp
  HttpClientModule.hpp
  HttpsClientModule.hpp
)

#Client
add_library(NoneIpClient
OBJECT
  HttpClientModule.cpp
  HttpsClientModule.cpp
)

target_include_directories(Http INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneIpClient PRIVATE abstractionLayer)
target_link_libraries(NoneIpClient PRIVATE Http)
target_link_libraries(NoneIpClient PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneIpClient)

#Server
add_library(NoneIpServer
OBJECT
  HttpServerModule.cpp
)

target_link_libraries(NoneIpServer PRIVATE abstractionLayer)
target_link_libraries(NoneIpServer PRIVATE Http)
target_link_libraries(NoneIpServer PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneIpServer)

target_compile_options(NoneIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(NoneIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
target_compile_definitions(NoneIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Http,INTERFACE_INCLUDE_DIRECTORIES>)
endif()