target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientModule.hpp
  IpServerModule.hpp
  HttpServerModule.hpp
  HttpClientModule.hpp
  HttpsClientModule.hpp
)

#Client
add_library(Cc32xxIpClient
OBJECT
  IpClientModule.cpp
  HttpClientModule.cpp
  HttpsClientModule.cpp
)

target_include_directories(IpClient INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Cc32xxIpClient PRIVATE abstractionLayer)
target_include_directories(Cc32xxIpClient PRIVATE IpClient)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxIpClient)

target_include_directories(Cc32xxIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Cc32xxIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

#Server
add_library(Cc32xxIpServer
OBJECT
  IpServerModule.cpp
  HttpServerModule.cpp
)

target_include_directories(IpServer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Cc32xxIpServer PRIVATE abstractionLayer)
target_link_libraries(Cc32xxIpServer PRIVATE IpServer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxIpServer)

target_include_directories(Cc32xxIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Cc32xxIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)