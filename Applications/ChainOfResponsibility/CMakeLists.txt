target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  CommandObject.hpp
  ChainOfResponsibility.hpp
)

add_library(ChainOfResponsibility STATIC
  ChainOfResponsibility.cpp
)

target_include_directories(ChainOfResponsibility INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(ChainOfResponsibility PUBLIC abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ChainOfResponsibility)

target_include_directories(ChainOfResponsibility PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(ChainOfResponsibility PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(ChainOfResponsibility PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:ChainOfResponsibility,INTERFACE_INCLUDE_DIRECTORIES>)
endif()