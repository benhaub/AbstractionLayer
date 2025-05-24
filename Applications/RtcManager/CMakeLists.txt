target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  RtcManager.hpp
)

add_library(RtcManager
OBJECT
  RtcManager.cpp
)

target_include_directories(RtcManager PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(RtcManager PRIVATE abstractionLayer)
target_link_libraries(RtcManager PRIVATE Rtc)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE RtcManager)

target_include_directories(RtcManager PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(RtcManager PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(RtcManager PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()