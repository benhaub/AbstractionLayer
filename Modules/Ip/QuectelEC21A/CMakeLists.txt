target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpCellularClientModule.hpp
  IpCellularServerModule.hpp
)
#Client
add_library(QuectelEC21AIpCellularClient
STATIC
  IpCellularClientModule.cpp
)

target_link_libraries(QuectelEC21AIpCellularClient PUBLIC abstractionLayer)
target_link_libraries(QuectelEC21AIpCellularClient PUBLIC IpClient)
target_link_libraries(QuectelEC21AIpCellularClient PUBLIC Logging)
target_link_libraries(QuectelEC21AIpCellularClient PUBLIC Utilities)
target_link_libraries(QuectelEC21AIpCellularClient PUBLIC QuectelEC21ACellular)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC QuectelEC21AIpCellularClient)

#Server
add_library(QuectelEC21AIpCellularServer
STATIC
  IpCellularServerModule.cpp
)

target_link_libraries(QuectelEC21AIpCellularServer PUBLIC abstractionLayer)
target_link_libraries(QuectelEC21AIpCellularServer PUBLIC IpClient)
target_link_libraries(QuectelEC21AIpCellularServer PUBLIC Logging)
target_link_libraries(QuectelEC21AIpCellularServer PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC QuectelEC21AIpCellularServer)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(QuectelEC21AIpCellularServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(QuectelEC21AIpCellularClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)