target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  HBridge.hpp
)

add_library(TexasInstrumentsDrv8872HBridge
OBJECT
  HBridge.cpp
)

target_include_directories(HBridge INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(TexasInstrumentsDrv8872HBridge PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE TexasInstrumentsDrv8872HBridge)

target_include_directories(TexasInstrumentsDrv8872HBridge PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(TexasInstrumentsDrv8872HBridge PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(TexasInstrumentsDrv8872HBridge PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)