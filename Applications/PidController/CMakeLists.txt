target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PidController.hpp
)

add_library(PidController
OBJECT
  PidController.cpp
)

target_include_directories(PidController PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(PidController PRIVATE abstractionLayer)
target_link_libraries(PidController PRIVATE IpClient)
target_link_libraries(PidController PRIVATE Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE PidController)

target_compile_options(PidController PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(PidController PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()