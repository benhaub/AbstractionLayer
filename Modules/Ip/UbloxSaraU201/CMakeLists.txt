target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpCellularClientModule.hpp
  IpCellularServerModule.hpp
)
#Client
add_library(UbloxSaraU201IpCellularClient
OBJECT
  IpCellularClientModule.cpp
)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC UbloxSaraU201IpCellularClient)

#Server
add_library(UbloxSaraU201IpCellularServer
OBJECT
  IpCellularServerModule.cpp
)

target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE UbloxSaraU201IpCellularServer)

target_include_directories(UbloxSaraU201IpCellularClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)
target_include_directories(UbloxSaraU201IpCellularServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)
target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(UbloxSaraU201IpCellularClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_options(UbloxSaraU201IpCellularServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
