target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  LoggerModule.hpp
)

add_library(EspLogger
OBJECT
  LoggerModule.cpp
)

target_include_directories(Logging INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Logging INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)
target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Logging,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(EspLogger PRIVATE abstractionLayer)
target_link_libraries(EspLogger PRIVATE Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspLogger)

target_compile_options(EspLogger PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspLogger PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)