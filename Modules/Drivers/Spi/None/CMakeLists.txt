target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  SpiModule.hpp
)

add_library(NoneSpi
STATIC
  SpiModule.cpp
)
target_include_directories(Spi INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneSpi PUBLIC abstractionLayer)
target_link_libraries(NoneSpi PUBLIC Spi)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NoneSpi)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneSpi PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
