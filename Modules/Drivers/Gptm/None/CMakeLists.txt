target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  GptmCaptureModule.hpp
  GptmPwmModule.hpp
  GptmTimerModule.hpp
)

add_library(NoneGptm
OBJECT
  GptmCaptureModule.cpp
  GptmPwmModule.cpp
  GptmTimerModule.cpp
)
target_include_directories(Gptm INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneGptm PRIVATE Utilities)
target_link_libraries(NoneGptm PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneGptm)

target_compile_options(NoneGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneGptm PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Gptm,INTERFACE_INCLUDE_DIRECTORIES>)
endif()