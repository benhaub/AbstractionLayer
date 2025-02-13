target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  Log.hpp
  StatusLogger.hpp
)

add_library(StatusLogger STATIC
  StatusLogger.cpp
)

target_include_directories(Logging INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Logging INTERFACE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC Logging)
target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(StatusLogger PUBLIC abstractionLayer)
target_link_libraries(StatusLogger PUBLIC Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC StatusLogger)
target_compile_options(StatusLogger PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)