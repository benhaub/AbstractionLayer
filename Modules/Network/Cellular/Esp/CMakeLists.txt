target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  CellularModule.hpp
)

add_library(EspCellular
OBJECT
  CellularModule.cpp
  command/sock_dce.cpp
  sock_commands_sim7600.cpp
)

target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Network,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(Network INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Network INTERFACE ${CMAKE_CURRENT_LIST_DIR}/command)
target_include_directories(Network INTERFACE ${CMAKE_CURRENT_LIST_DIR}/generic_module)

target_link_libraries(Network INTERFACE abstractionLayer)
target_link_libraries(EspCellular PRIVATE Network)

target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspCellular)

target_compile_options(EspCellular PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspCellular PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
