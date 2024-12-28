target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientModule.hpp
  IpServerModule.hpp
  HttpServerModule.hpp
)
#Client
add_library(Cc32xxIpClient
STATIC
  IpClientModule.cpp
)

target_link_libraries(Cc32xxIpClient PUBLIC abstractionLayer)
target_link_libraries(Cc32xxIpClient PUBLIC IpClient)
target_link_libraries(Cc32xxIpClient PUBLIC Logging)
target_link_libraries(Cc32xxIpClient PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC Cc32xxIpClient)

#Server
add_library(Cc32xxIpServer
STATIC
  IpServerModule.cpp
  HttpServerModule.cpp
)

target_link_libraries(Cc32xxIpServer PUBLIC abstractionLayer)
target_link_libraries(Cc32xxIpServer PUBLIC IpClient)
target_link_libraries(Cc32xxIpServer PUBLIC Logging)
target_link_libraries(Cc32xxIpServer PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC Cc32xxIpServer)

target_include_directories(Cc32xxIpServer PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)
target_include_directories(Cc32xxIpClient PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)
target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(Cc32xxIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(Cc32xxIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)