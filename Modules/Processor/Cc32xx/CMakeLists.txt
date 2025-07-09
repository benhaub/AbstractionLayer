target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  ProcessorModule.hpp
)

add_library(Cc32xxProcessor
OBJECT
  ProcessorModule.cpp
)

target_include_directories(Processor INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Cc32xxProcessor PRIVATE ${CMAKE_CURRENT_LIST_DIR}/CMSIS_6/CMSIS/Core/Include)
target_include_directories(Cc32xxProcessor PRIVATE ${CMAKE_CURRENT_LIST_DIR}/CMSIS_6/CMSIS/Core/Include/m-profile)

target_link_libraries(Cc32xxProcessor PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxProcessor)

target_include_directories(Cc32xxProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Cc32xxProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxProcessor PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

#Don't inlcude the DSP functions from CMSIS. I might be missing a library that links the DSP calls.
target_compile_definitions(Cc32xxProcessor PRIVATE -D__ARM_FEATURE_DSP=0)
target_compile_options(Cc32xxProcessor PRIVATE -fpermissive)
