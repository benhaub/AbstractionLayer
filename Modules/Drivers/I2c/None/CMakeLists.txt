target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  I2cModule.hpp
)

add_library(NoneI2c
STATIC
  I2cModule.cpp
)
target_include_directories(I2c INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneI2c PUBLIC Utilities)
target_link_libraries(NoneI2c PUBLIC abstractionLayer)

target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneI2c)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneI2c PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:I2c,INTERFACE_INCLUDE_DIRECTORIES>)
endif()