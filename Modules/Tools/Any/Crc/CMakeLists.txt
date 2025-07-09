target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  CyclicRedundancyCheckModule.hpp
  crc.h
)

add_library(AnyCyclicRedundancyCheck
OBJECT
  CyclicRedundancyCheckModule.cpp
  crc.c
)

target_link_libraries(AnyCyclicRedundancyCheck PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC AnyCyclicRedundancyCheck)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(AnyCyclicRedundancyCheck PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(AnyCyclicRedundancyCheck PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
