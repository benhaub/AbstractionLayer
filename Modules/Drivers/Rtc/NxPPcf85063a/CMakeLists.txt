target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  RtcModule.hpp
)

add_library(NxpPcf85063aRtc
STATIC
  RtcModule.cpp
)
target_include_directories(Rtc INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NxpPcf85063aRtc PUBLIC abstractionLayer)
target_link_libraries(NxpPcf85063aRtc PUBLIC I2c)
target_link_libraries(NxpPcf85063aRtc PUBLIC Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NxpPcf85063aRtc)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NxpPcf85063aRtc PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)