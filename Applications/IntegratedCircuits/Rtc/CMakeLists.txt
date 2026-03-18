target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  NxpPcf8506a/Nxppcf8506aModule.hpp
)

add_library(NxpPcf8506a
OBJECT
  NxpPcf8506a/Nxppcf8506aModule.cpp
)

target_include_directories(NxpPcf8506a INTERFACE ${CMAKE_CURRENT_LIST_DIR})
#target_include_directories(NxpPcf8506a PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(NxpPcf8506a PRIVATE abstractionLayer)
target_link_libraries(NxpPcf8506a PUBLIC I2c)
target_link_libraries(NxpPcf8506a PRIVATE Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NxpPcf8506a)

target_compile_options(NxpPcf8506a PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NxpPcf8506a PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()