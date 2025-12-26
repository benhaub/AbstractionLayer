target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  Inference.hpp
)

#https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/docs/new_platform_support.md
set(TFLITE_CREATE_TREE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/tflite-micro/createTree.sh")
file(WRITE "${TFLITE_CREATE_TREE_SCRIPT}" "
  #!/bin/bash
  if [ ! -d ../TensorFlowLiteTree ]; then
      echo \"Creating TensorFlowLite tree\"
      python3 tensorflow/lite/micro/tools/project_generation/create_tflm_tree.py -e hello_world -e micro_speech -e person_detection ../TensorFlowLiteTree 
      cp tensorflow/lite/array.* ../TensorFlowLiteTree/tensorflow/lite/
      cp -r third_party/ruy ../TensorFlowLiteTree/third_party/ruy
  fi
")
execute_process(
  COMMAND
    chmod +x "${TFLITE_CREATE_TREE_SCRIPT}"
  COMMAND
    bash "${TFLITE_CREATE_TREE_SCRIPT}"
  WORKING_DIRECTORY
    "${CMAKE_CURRENT_LIST_DIR}/tflite-micro"
  COMMAND_ERROR_IS_FATAL
    ANY
)
file(REMOVE "${TFLITE_CREATE_TREE_SCRIPT}")

file(GLOB_RECURSE TFLITE_SOURCES 
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/tensorflow/*.cc"
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/tensorflow/*.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/tensorflow/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/signal/*.cc"
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/signal/*.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/signal/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/third_party/*.cc"
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/third_party/*.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/third_party/*.c"
)

add_library(TensorFlowLiteInference
OBJECT
  Inference.cpp
  ${TFLITE_SOURCES}
)

target_include_directories(MlInference INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(TensorFlowLiteInference PUBLIC ${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree)
target_include_directories(TensorFlowLiteInference PRIVATE ${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/third_party/gemmlowp)
target_include_directories(TensorFlowLiteInference PUBLIC ${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/third_party/flatbuffers/include)
target_include_directories(TensorFlowLiteInference PRIVATE ${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/third_party/kissfft)
target_include_directories(TensorFlowLiteInference PRIVATE ${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/third_party/ruy)
target_include_directories(TensorFlowLiteInference PUBLIC ${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/tensorflow/lite/micro)
target_include_directories(TensorFlowLiteInference PRIVATE ${CMAKE_CURRENT_LIST_DIR}/TensorFlowLiteTree/tensorflow/compiler/mlir/lite/schema)

target_link_libraries(TensorFlowLiteInference PRIVATE abstractionLayer)
target_link_libraries(TensorFlowLiteInference PRIVATE MlInference)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE TensorFlowLiteInference)

target_compile_options(TensorFlowLiteInference
  PRIVATE
    $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>
    -Wno-sign-compare
)
target_compile_definitions(TensorFlowLiteInference PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Network,INTERFACE_INCLUDE_DIRECTORIES>)
endif()