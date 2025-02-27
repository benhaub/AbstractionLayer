target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  28byj485V.hpp
)

add_library(28byj485V STATIC
  28byj485V.cpp
)

target_include_directories(28byj485V PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(28byj485V PUBLIC abstractionLayer)
target_link_libraries(28byj485V PUBLIC Gptm)
target_link_libraries(28byj485V PUBLIC Pwm)
target_link_libraries(28byj485V PUBLIC OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC 28byj485V)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(28byj485V PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)