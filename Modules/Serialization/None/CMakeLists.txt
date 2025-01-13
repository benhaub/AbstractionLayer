target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  SerializationModule.hpp
)

add_library(NoneSerialization
STATIC
  SerializationModule.cpp
)

target_include_directories(Serialization INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneSerialization PUBLIC abstractionLayer)
target_link_libraries(NoneSerialization PUBLIC Serialization)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneSerialization)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneSerialization PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
