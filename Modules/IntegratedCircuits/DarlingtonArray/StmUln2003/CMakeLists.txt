target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  DarlingtonArray.hpp
)

add_library(NoneDarlingtonArray
OBJECT
  DarlingtonArray.cpp
)

target_include_directories(DarlingtonArray INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneDarlingtonArray PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneDarlingtonArray)

target_include_directories(NoneDarlingtonArray PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(NoneDarlingtonArray PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneDarlingtonArray PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)