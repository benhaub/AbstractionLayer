target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  SpiModule.hpp
)

add_library(Cc32xxSpi
OBJECT
  SpiModule.cpp
)

target_include_directories(Spi INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Cc32xxSpi PRIVATE abstractionLayer)
target_link_libraries(Cc32xxSpi PRIVATE Spi)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxSpi)

target_include_directories(Cc32xxSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Cc32xxSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
