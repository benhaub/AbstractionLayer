target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  GpioModule.hpp
)

add_library(NoneGpio
STATIC
  GpioModule.cpp
)

target_include_directories(Gpio INTERFACE ${CMAKE_CURRENT_LIST_DIR})
#For AbstractionLayer Applications that use the GPTM, this interface inlcude gives access to driver headers that the application has linked with.
target_include_directories(Gpio INTERFACE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(NoneGpio PUBLIC abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneGpio)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneGpio PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneGpio PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Gpio,INTERFACE_INCLUDE_DIRECTORIES>)
endif()
