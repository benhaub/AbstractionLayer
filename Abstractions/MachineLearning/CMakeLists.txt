target_sources(abstractionLayer
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  InferenceAbstraction.hpp
)

add_library(MlInference INTERFACE)

target_include_directories(MlInference INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(MlInference INTERFACE Logging)