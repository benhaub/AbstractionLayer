target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientModule.hpp
  IpServerModule.hpp
  HttpServerModule.hpp
)
#Client
add_library(EspIpClient
STATIC
  IpClientModule.cpp
)

target_include_directories(IpServer INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)
target_include_directories(IpClient INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)

target_link_libraries(EspIpClient PUBLIC abstractionLayer)
target_link_libraries(EspIpClient PUBLIC IpClient)
target_link_libraries(EspIpClient PUBLIC Logging)
target_link_libraries(EspIpClient PUBLIC Utilities)
target_link_libraries(EspIpClient PUBLIC OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC EspIpClient)

#Server
add_library(EspIpServer
STATIC
  IpServerModule.cpp
  HttpServerModule.cpp
)

target_link_libraries(EspIpServer PUBLIC abstractionLayer)
target_link_libraries(EspIpServer PUBLIC IpClient)
target_link_libraries(EspIpServer PUBLIC Logging)
target_link_libraries(EspIpServer PUBLIC Utilities)
target_link_libraries(EspIpServer PUBLIC OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC EspIpServer)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(EspIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(EspIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)