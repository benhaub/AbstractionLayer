target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PwmModule.hpp
)

add_library(NonePwm
OBJECT
  PwmModule.cpp
)

target_include_directories(Pwm INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NonePwm PRIVATE abstractionLayer)
target_link_libraries(NonePwm PRIVATE Pwm)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NonePwm)

target_compile_options(NonePwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NonePwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Pwm,INTERFACE_INCLUDE_DIRECTORIES>)
endif()