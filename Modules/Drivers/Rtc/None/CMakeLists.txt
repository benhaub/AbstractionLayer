target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  RtcModule.hpp
)

add_library(NoneRtc
STATIC
  RtcModule.cpp
)
target_include_directories(Rtc INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneRtc PUBLIC Utilities)
target_link_libraries(NoneRtc PUBLIC abstractionLayer)

target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneRtc)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneRtc PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Rtc,INTERFACE_INCLUDE_DIRECTORIES>)
endif()