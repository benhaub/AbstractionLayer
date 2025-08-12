target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  HBridgeAbstraction.hpp
  TiDrv8872/TiDrv8872.hpp
)

add_library(TexasInstrumentsDrv8872HBridge
OBJECT
  TiDrv8872/TiDrv8872.cpp
)

target_link_libraries(TexasInstrumentsDrv8872HBridge PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE TexasInstrumentsDrv8872HBridge)

target_include_directories(TexasInstrumentsDrv8872HBridge PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(TexasInstrumentsDrv8872HBridge PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(TexasInstrumentsDrv8872HBridge PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()