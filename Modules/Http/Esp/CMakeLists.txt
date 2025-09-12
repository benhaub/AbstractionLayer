target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  HttpServerModule.hpp
  HttpClientModule.hpp
  HttpsClientModule.hpp
)
#Client
add_library(EspIpClient
OBJECT
  HttpClientModule.cpp
  HttpsClientModule.cpp
)

target_include_directories(Http INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Http INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)
target_include_directories(Http INTERFACE $<TARGET_PROPERTY:Protocols,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:IpClient,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(EspIpClient PRIVATE abstractionLayer)
target_link_libraries(EspIpClient PRIVATE Http)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspIpClient)

target_compile_options(EspIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

#Server
add_library(EspIpServer
OBJECT
  HttpServerModule.cpp
)

target_link_libraries(EspIpServer PRIVATE abstractionLayer)
target_link_libraries(EspIpServer PRIVATE IpServer)
target_link_libraries(EspIpServer PRIVATE Utilities)
target_link_libraries(EspIpServer PRIVATE OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspIpServer)

target_compile_options(EspIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
