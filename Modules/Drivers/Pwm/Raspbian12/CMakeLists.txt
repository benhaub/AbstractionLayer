target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PwmModule.hpp
)

add_library(Raspbian12Pwm
STATIC
  PwmModule.cpp
)

target_include_directories(Pwm INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Raspbian12Pwm PRIVATE abstractionLayer)
target_link_libraries(Raspbian12Pwm PRIVATE Pwm)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Raspbian12Pwm)

target_compile_options(Raspbian12Pwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Raspbian12Pwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
target_compile_definitions(Raspbian12Pwm
PRIVATE
  PATH_TO_SCRIPTS="${CMAKE_CURRENT_LIST_DIR}/Scripts"
)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Pwm,INTERFACE_INCLUDE_DIRECTORIES>)
endif()