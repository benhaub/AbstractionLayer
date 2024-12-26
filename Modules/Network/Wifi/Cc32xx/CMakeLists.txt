target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  WifiModule.hpp
)

add_library(Cc32xxWifi
STATIC
  WifiModule.cpp
)
target_include_directories(Network INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Cc32xxWifi PUBLIC abstractionLayer)
target_link_libraries(Cc32xxWifi PUBLIC Network)
target_link_libraries(Cc32xxWifi PUBLIC Cc32xxSpi)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC Cc32xxWifi)

target_include_directories(Cc32xxWifi PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)
target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(Cc32xxWifi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
