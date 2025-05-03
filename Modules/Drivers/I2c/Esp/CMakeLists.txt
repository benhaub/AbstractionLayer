target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  I2cModule.hpp
)

add_library(EspI2c
OBJECT
  I2cModule.cpp
)

target_include_directories(I2c INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(I2c INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)
target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:I2c,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(EspI2c PRIVATE abstractionLayer)
target_link_libraries(EspI2c PRIVATE Gpio)
target_link_libraries(EspI2c PRIVATE Logging)
target_link_libraries(EspI2c PRIVATE OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspI2c)

target_compile_options(EspI2c PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspI2c PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
