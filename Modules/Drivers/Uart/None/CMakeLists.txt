target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  UartModule.hpp
)

add_library(NoneUart
STATIC
  UartModule.cpp
)
target_include_directories(Uart INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneUart PUBLIC abstractionLayer)
target_link_libraries(NoneUart PUBLIC Uart)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneUart)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneUart PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Uart,INTERFACE_INCLUDE_DIRECTORIES>)
endif()