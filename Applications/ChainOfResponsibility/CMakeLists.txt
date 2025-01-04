target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  CommandObject.hpp
  ChainOfResponsibility.hpp
)

add_library(ChainOfResponsibility STATIC
  ChainOfResponsibility.cpp
)

target_include_directories(ChainOfResponsibility INTERFACE ChainOfResponsibility)
target_link_libraries(ChainOfResponsibility PUBLIC abstractionLayer)
target_link_libraries(ChainOfResponsibility PUBLIC Utilities)
target_link_libraries(ChainOfResponsibility PUBLIC OperatingSystem)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ChainOfResponsibility)

if (ESP_PLATFORM)
  target_include_directories(ChainOfResponsibility PRIVATE $<TARGET_PROPERTY:__idf_main,INTERFACE_INCLUDE_DIRECTORIES>)
  target_include_directories(__idf_main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
else()
  target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
endif()

target_compile_options(ChainOfResponsibility PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
