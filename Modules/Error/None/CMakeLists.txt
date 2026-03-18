add_library(NoneError
OBJECT
  ErrorModule.cpp
)

target_link_libraries(NoneError PRIVATE Utilities)
target_include_directories(Network INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Gpio INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Uart INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Storage INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(OperatingSystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneError)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneError PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Utilities,INTERFACE_INCLUDE_DIRECTORIES>)
endif()