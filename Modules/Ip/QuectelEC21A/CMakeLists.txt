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

target_link_libraries(QuectelEC21AIpCellularClient PRIVATE abstractionLayer)
target_link_libraries(QuectelEC21AIpCellularClient PRIVATE IpClient)
target_link_libraries(QuectelEC21AIpCellularClient PRIVATE Utilities)
target_link_libraries(QuectelEC21AIpCellularClient PRIVATE QuectelEC21ACellular)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE QuectelEC21AIpCellularClient)

#Server
add_library(QuectelEC21AIpCellularServer
STATIC
  IpCellularServerModule.cpp
)

target_link_libraries(QuectelEC21AIpCellularServer PRIVATE abstractionLayer)
target_link_libraries(QuectelEC21AIpCellularServer PRIVATE IpClient)
target_link_libraries(QuectelEC21AIpCellularServer PRIVATE Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE QuectelEC21AIpCellularServer)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(QuectelEC21AIpCellularServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(QuectelEC21AIpCellularClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)