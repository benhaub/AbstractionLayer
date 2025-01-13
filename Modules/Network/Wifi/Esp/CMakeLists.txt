target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  WifiModule.hpp
)

add_library(EspWifi
STATIC
  WifiModule.cpp
)

target_include_directories(Network INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Network INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)

target_link_libraries(EspWifi PUBLIC abstractionLayer)
target_link_libraries(EspWifi PUBLIC Network)
target_link_libraries(EspWifi PUBLIC Utilities)
target_link_libraries(EspWifi PUBLIC Logging)
target_link_libraries(EspWifi PUBLIC OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC EspWifi)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(EspWifi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
