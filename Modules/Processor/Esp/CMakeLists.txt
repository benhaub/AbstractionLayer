target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  ProcessorModule.hpp
)

add_library(EspProcessor
OBJECT
  ProcessorModule.cpp
)

target_include_directories(Processor INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Processor,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(Processor INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)

target_link_libraries(EspProcessor PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspProcessor)

target_include_directories(EspProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(EspProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
