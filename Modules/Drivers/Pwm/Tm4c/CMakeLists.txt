target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PwmModule.hpp
)

add_library(Tm4cPwm
OBJECT
  PwmModule.cpp
)

target_include_directories(Pwm INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Pwm INTERFACE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(Tm4cPwm PRIVATE abstractionLayer)
target_link_libraries(Tm4cPwm PRIVATE Pwm)
target_link_libraries(Tm4cPwm PRIVATE driver)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Tm4cPwm)

target_compile_options(Tm4cPwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Tm4cPwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)