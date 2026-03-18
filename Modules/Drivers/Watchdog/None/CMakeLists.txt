target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  WatchdogModule.hpp
)

add_library(NoneWatchdog
OBJECT
  WatchdogModule.cpp
)

target_include_directories(Watchdog INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneWatchdog PRIVATE abstractionLayer)
target_link_libraries(NoneWatchdog PRIVATE Watchdog)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneWatchdog)

target_compile_options(NoneWatchdog PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneWatchdog PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Watchdog,INTERFACE_INCLUDE_DIRECTORIES>)
endif()