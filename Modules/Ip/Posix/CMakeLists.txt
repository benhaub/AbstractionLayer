target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientModule.hpp
  IpServerModule.hpp
  HttpServerModule.hpp
)
#Client
add_library(PosixIpClient
STATIC
  IpClientModule.cpp
)

target_link_libraries(PosixIpClient PUBLIC abstractionLayer)
target_link_libraries(PosixIpClient PUBLIC Network)
target_link_libraries(PosixIpClient PUBLIC OperatingSystem)
target_link_libraries(PosixIpClient PUBLIC Utilities)
target_link_libraries(PosixIpClient PUBLIC Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC PosixIpClient)

#Server
add_library(PosixIpServer
STATIC
  IpServerModule.cpp
  HttpServerModule.cpp
)

target_link_libraries(PosixIpServer PUBLIC abstractionLayer)
target_link_libraries(PosixIpServer PUBLIC Network)
target_link_libraries(PosixIpServer PUBLIC OperatingSystem)
target_link_libraries(PosixIpServer PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC PosixIpServer)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
else()
  target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
endif()

target_compile_options(PosixIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(PosixIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
