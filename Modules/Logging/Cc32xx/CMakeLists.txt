target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  LoggerModule.hpp
)

add_library(Cc32xxLogger
OBJECT
  LoggerModule.cpp
)

target_include_directories(Logging INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Logging INTERFACE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(Cc32xxLogger PRIVATE abstractionLayer)
target_link_libraries(Cc32xxLogger PRIVATE Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxLogger)

target_compile_options(Cc32xxLogger PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxLogger PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)