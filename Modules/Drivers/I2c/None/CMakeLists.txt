target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  I2cModule.hpp
)

add_library(NoneI2c
OBJECT
  I2cModule.cpp
)

target_include_directories(NoneI2c INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneI2c PRIVATE abstractionLayer)
target_link_libraries(NoneI2c PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneI2c)

target_compile_options(NoneI2c PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneI2c PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:I2c,INTERFACE_INCLUDE_DIRECTORIES>)
endif()