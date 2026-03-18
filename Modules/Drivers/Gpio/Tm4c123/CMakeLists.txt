target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  GpioModule.hpp
)

add_library(Tm4c123Gpio
OBJECT
  GpioModule.cpp
)

target_include_directories(Gpio INTERFACE ${CMAKE_CURRENT_LIST_DIR})
#For AbstractionLayer Applications that use the GPTM, this interface inlcude gives access to driver headers that the application has linked with.
target_include_directories(Gpio INTERFACE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(Tm4c123Gpio PRIVATE abstractionLayer)
target_link_libraries(Tm4c123Gpio PRIVATE Gpio)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Tm4c123Gpio)

target_compile_options(Tm4c123Gpio PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Tm4c123Gpio PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)