add_library(EspMemoryConfig
OBJECT
  MemoryConfigModule.cpp
)

target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspMemoryConfig)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(EspMemoryConfig PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(EspMemoryConfig PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
