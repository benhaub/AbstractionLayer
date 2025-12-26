target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  SpiModule.hpp
)

add_library(Tm4cSpi
OBJECT
  SpiModule.cpp
)

target_include_directories(Spi INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Tm4cSpi PRIVATE abstractionLayer)
target_link_libraries(Tm4cSpi PRIVATE Spi)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Tm4cSpi)

target_include_directories(Tm4cSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Tm4cSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Tm4cSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Spi,INTERFACE_INCLUDE_DIRECTORIES>)
endif()