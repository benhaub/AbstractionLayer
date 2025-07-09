target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  RtcModule.hpp
)

add_library(Cc32xxRtc
OBJECT
  RtcModule.cpp
)

target_include_directories(Rtc INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Rtc INTERFACE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(Cc32xxRtc PRIVATE abstractionLayer)
target_link_libraries(Cc32xxRtc PRIVATE Rtc)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxRtc)

target_compile_options(Cc32xxRtc PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxRtc PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)