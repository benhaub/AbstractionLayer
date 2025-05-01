target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  ProcessorModule.hpp
)

add_library(NoneProcessor
OBJECT
  ProcessorModule.cpp
)

target_include_directories(Processor INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneProcessor PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneProcessor)

target_include_directories(NoneProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(NoneProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
