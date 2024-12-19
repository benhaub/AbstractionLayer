target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  GptmCaptureModule.hpp
  GptmPwmModule.hpp
  GptmTimerModule.hpp
)

add_library(Cc32xxGptm
STATIC
  GptmCaptureModule.cpp
  GptmPwmModule.cpp
  GptmTimerModule.cpp
)
target_include_directories(Gptm INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Cc32xxGptm PUBLIC Utilities)
target_link_libraries(Cc32xxGptm PUBLIC abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC Cc32xxGptm)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Cc32xxGptm PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Cc32xxGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)