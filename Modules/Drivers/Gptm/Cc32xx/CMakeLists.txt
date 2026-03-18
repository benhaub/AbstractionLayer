target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  GptmCaptureModule.hpp
  GptmPwmModule.hpp
  GptmTimerModule.hpp
)

add_library(Cc32xxGptm
OBJECT
  GptmCaptureModule.cpp
  GptmPwmModule.cpp
  GptmTimerModule.cpp
)

target_include_directories(Gptm INTERFACE ${CMAKE_CURRENT_LIST_DIR})
#For AbstractionLayer Applications that use the GPTM, this interface inlcude gives access to driver headers that the application has linked with.
target_include_directories(Gptm INTERFACE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)
target_include_directories(Cc32xxGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(Cc32xxGptm PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxGptm)

target_compile_options(Cc32xxGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)