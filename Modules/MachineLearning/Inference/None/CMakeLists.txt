target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  Inference.hpp
)

add_library(TensorFlowLiteInference
OBJECT
  Inference.cpp
)

target_include_directories(MlInference INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(TensorFlowLiteInference PRIVATE abstractionLayer)
target_link_libraries(TensorFlowLiteInference PRIVATE Network)
target_link_libraries(TensorFlowLiteInference PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE TensorFlowLiteInference)

target_compile_options(TensorFlowLiteInference PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(TensorFlowLiteInference PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Network,INTERFACE_INCLUDE_DIRECTORIES>)
endif()