add_subdirectory(MbedTls)

target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  HttpServerModule.hpp
  HttpClientModule.hpp
  HttpsClientModule.hpp
  MbedTlsCompatibility/SendReceive.hpp
)

#Client
add_library(PosixIpClient
OBJECT
  HttpClientModule.cpp
  HttpsClientModule.cpp
  MbedTlsCompatibility/SendReceive.cpp
)

target_include_directories(Http INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(PosixIpClient PRIVATE abstractionLayer)
target_link_libraries(PosixIpClient PRIVATE IpClient)
target_link_libraries(PosixIpClient PRIVATE Network)
target_link_libraries(PosixIpClient PRIVATE OperatingSystem)
target_link_libraries(PosixIpClient PRIVATE Event)
target_link_libraries(PosixIpClient PUBLIC MbedTls)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE PosixIpClient)

target_compile_options(PosixIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(PosixIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

#Server
add_library(PosixIpServer
OBJECT
  HttpServerModule.cpp
)

target_link_libraries(PosixIpServer PRIVATE abstractionLayer)
target_link_libraries(PosixIpServer PRIVATE IpServer)
target_link_libraries(PosixIpServer PRIVATE Network)
target_link_libraries(PosixIpServer PRIVATE OperatingSystem)
target_link_libraries(PosixIpServer PRIVATE Event)
target_link_libraries(PosixIpServer PUBLIC MbedTls)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE PosixIpServer)

target_compile_options(PosixIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(PosixIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)