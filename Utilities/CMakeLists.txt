target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  Global.hpp
  Types.hpp
  Error.hpp
  Math.hpp
  Algorithm.hpp
)

add_library(Utilities INTERFACE)

target_include_directories(Utilities INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(abstractionLayer INTERFACE Utilities)