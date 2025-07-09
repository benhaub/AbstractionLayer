target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  Sm10001.hpp
)

add_library(Sm10001
OBJECT
  Sm10001.cpp
)

target_include_directories(Sm10001 PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Sm10001 PRIVATE abstractionLayer)
target_link_libraries(Sm10001 PRIVATE Gptm)
target_link_libraries(Sm10001 PRIVATE Gpio)
target_link_libraries(Sm10001 PRIVATE Pwm)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Sm10001)

target_compile_options(Sm10001 PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Sm10001 PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()