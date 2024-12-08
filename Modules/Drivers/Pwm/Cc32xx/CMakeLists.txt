target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PwmModule.hpp
)

add_library(Cc32xxPwm
STATIC
  PwmModule.cpp
)
target_include_directories(Pwm INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Cc32xxPwm PUBLIC abstractionLayer)
target_link_libraries(Cc32xxPwm PUBLIC Pwm)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC Cc32xxPwm)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Cc32xxPwm PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Cc32xxPwm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
