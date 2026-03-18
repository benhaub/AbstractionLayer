target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  GptmCaptureModule.hpp
  GptmPwmModule.hpp
  GptmTimerModule.hpp
)

add_library(EspGptm
OBJECT
  GptmCaptureModule.cpp
  GptmPwmModule.cpp
  GptmTimerModule.cpp
)

target_include_directories(Gptm INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Gptm INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)
target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(EspGptm PRIVATE abstractionLayer)
target_link_libraries(EspGptm PRIVATE Gptm)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspGptm)

target_compile_options(EspGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)