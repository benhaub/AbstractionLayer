target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  NetworkFactory.hpp
  IcCommFactory.hpp
)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()