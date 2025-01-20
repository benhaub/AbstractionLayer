target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  SignalsAndSlots.hpp
)

add_library(SignalsAndSlots STATIC
  SignalsAndSlots.cpp
)

target_include_directories(SignalsAndSlots PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(SignalsAndSlots PUBLIC OperatingSystem)
target_link_libraries(SignalsAndSlots PUBLIC Utilities)
target_link_libraries(SignalsAndSlots PUBLIC Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC SignalsAndSlots)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(SignalsAndSlots PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)