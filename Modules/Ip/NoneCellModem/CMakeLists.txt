target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpCellularClientModule.hpp
  IpCellularServerModule.hpp
)
#Client
add_library(NoneIpCellularClient
OBJECT
  IpCellularClientModule.cpp
)

target_include_directories(IpClient INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(IpClient INTERFACE abstractionLayer)
target_link_libraries(NoneIpCellularClient PRIVATE IpClient)
target_link_libraries(NoneIpCellularClient PRIVATE Network)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneIpCellularClient)

#Server
add_library(NoneIpCellularServer
OBJECT
  IpCellularServerModule.cpp
)

target_include_directories(IpServer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(IpServer INTERFACE abstractionLayer)
target_link_libraries(NoneIpCellularServer PRIVATE IpServer)
target_link_libraries(NoneIpCellularServer PRIVATE Network)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneIpCellularServer)

target_compile_options(NoneIpCellularServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(NoneIpCellularClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:NoneIpCellularServer,INTERFACE_INCLUDE_DIRECTORIES>)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:NoneIpCellularClient,INTERFACE_INCLUDE_DIRECTORIES>)
endif()