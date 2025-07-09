target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  AdcModule.hpp
)

add_library(EspAdc
OBJECT
  AdcModule.cpp
)

target_include_directories(Adc INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Adc INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)

target_link_libraries(EspAdc PRIVATE abstractionLayer)
target_link_libraries(EspAdc PRIVATE Adc)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspAdc)

target_compile_options(EspAdc PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspAdc PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)