target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PwmModule.hpp
)

add_library(Raspbian12Pi4ModelBPwm
STATIC
  PwmModule.cpp
)

target_include_directories(Pwm INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Raspbian12Pi4ModelBPwm PRIVATE abstractionLayer)
target_link_libraries(Raspbian12Pi4ModelBPwm PRIVATE Pwm)
target_link_libraries(Raspbian12Pi4ModelBPwm PRIVATE pigpio)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Raspbian12Pi4ModelBPwm)

target_compile_options(Raspbian12Pi4ModelBPwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Raspbian12Pi4ModelBPwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
target_compile_definitions(Raspbian12Pi4ModelBPwm
PRIVATE
  PATH_TO_SCRIPTS="${CMAKE_CURRENT_LIST_DIR}/Scripts"
)
