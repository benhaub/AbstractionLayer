target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  GptmCaptureModule.hpp
  GptmPwmModule.hpp
  GptmTimerModule.hpp
)

add_library(Raspbian12Pi4ModelBGptm
OBJECT
  GptmCaptureModule.cpp
  GptmPwmModule.cpp
  GptmTimerModule.cpp
)
target_include_directories(Gptm INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Raspbian12Pi4ModelBGptm PRIVATE Utilities)
target_link_libraries(Raspbian12Pi4ModelBGptm PRIVATE abstractionLayer)
target_link_libraries(Raspbian12Pi4ModelBGptm PRIVATE pigpio)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Raspbian12Pi4ModelBGptm)

target_compile_options(Raspbian12Pi4ModelBGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Raspbian12Pi4ModelBGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)