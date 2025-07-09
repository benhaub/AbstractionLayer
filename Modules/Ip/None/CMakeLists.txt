target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientModule.hpp
  IpServerModule.hpp
  HttpServerModule.hpp
  HttpClientModule.hpp
  HttpsClientModule.hpp
)
#Client
add_library(NoneIpClient
OBJECT
  IpClientModule.cpp
  HttpClientModule.cpp
  HttpsClientModule.cpp
)

target_link_libraries(NoneIpClient PRIVATE abstractionLayer)
target_link_libraries(NoneIpClient PRIVATE IpClient)
target_link_libraries(NoneIpClient PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneIpClient)

#Server
add_library(NoneIpServer
OBJECT
  IpServerModule.cpp
  HttpServerModule.cpp
)

target_link_libraries(NoneIpServer PRIVATE abstractionLayer)
target_link_libraries(NoneIpServer PRIVATE IpClient)
target_link_libraries(NoneIpServer PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneIpServer)

target_compile_options(NoneIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(NoneIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
target_compile_definitions(NoneIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:IpServer,INTERFACE_INCLUDE_DIRECTORIES>)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:IpClient,INTERFACE_INCLUDE_DIRECTORIES>)
endif()