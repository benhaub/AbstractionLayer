target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  LoggerModule.hpp
)

add_library(NoneLogger
STATIC
  LoggerModule.cpp
)

target_include_directories(Logging INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneLogger PUBLIC abstractionLayer)
target_link_libraries(NoneLogger PUBLIC Logging)
target_link_libraries(NoneLogger PUBLIC Utilities)
target_link_libraries(NoneLogger PUBLIC Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneLogger)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
else()
  target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
endif()

target_compile_options(NoneLogger PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
