target_sources(abstractionLayer
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientAbstraction.hpp
  IpServerAbstraction.hpp
  HttpServerAbstraction.hpp
  HttpClientAbstraction.hpp
)

add_library(IpClient INTERFACE)
add_library(IpServer INTERFACE)
add_library(HttpServer INTERFACE)

target_include_directories(IpClient INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(IpServer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(HttpServer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(IpClient INTERFACE Logging)
target_link_libraries(IpServer INTERFACE Logging)
target_link_libraries(HttpServer INTERFACE Logging)
