target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientModule.hpp
  IpServerModule.hpp
  HttpServerModule.hpp
)
#Client
add_library(NoneIpClient
STATIC
  IpClientModule.cpp
)

target_link_libraries(NoneIpClient PUBLIC abstractionLayer)
target_link_libraries(NoneIpClient PUBLIC IpClient)
target_link_libraries(NoneIpClient PUBLIC Logging)
target_link_libraries(NoneIpClient PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneIpClient)

#Server
add_library(NoneIpServer
STATIC
  IpServerModule.cpp
  HttpServerModule.cpp
)

target_link_libraries(NoneIpServer PUBLIC abstractionLayer)
target_link_libraries(NoneIpServer PUBLIC IpClient)
target_link_libraries(NoneIpServer PUBLIC Logging)
target_link_libraries(NoneIpServer PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneIpServer)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(NoneIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)