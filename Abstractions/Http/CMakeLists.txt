target_sources(abstractionLayer
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  HttpServerAbstraction.hpp
  HttpClientAbstraction.hpp
)

add_library(Http INTERFACE)

target_include_directories(Http INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Http INTERFACE Logging)