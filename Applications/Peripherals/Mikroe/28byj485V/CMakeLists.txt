target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  28byj485V.hpp
)

add_library(28byj485V
OBJECT
  28byj485V.cpp
)

target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(28byj485V PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(28byj485V PRIVATE abstractionLayer)
target_link_libraries(28byj485V PRIVATE Gptm)
target_link_libraries(28byj485V PRIVATE Pwm)
target_link_libraries(28byj485V PRIVATE OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE 28byj485V)

target_compile_options(28byj485V PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(28byj485V PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()