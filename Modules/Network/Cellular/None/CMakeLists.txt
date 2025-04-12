target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  CellularModule.hpp
)

add_library(NoneCellular
STATIC
  CellularModule.cpp
)

target_include_directories(NoneCellular INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(NoneCellular PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(NoneCellular PRIVATE abstractionLayer)
target_link_libraries(NoneCellular PRIVATE Network)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneCellular)

target_compile_options(NoneCellular PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneCellular PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Network,INTERFACE_INCLUDE_DIRECTORIES>)
endif()