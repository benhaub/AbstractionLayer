target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  OperatingSystemModule.hpp
)

add_library(DarwinOperatingSystem
STATIC
  OperatingSystemModule.cpp
)

target_include_directories(OperatingSystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(DarwinOperatingSystem PUBLIC abstractionLayer)
target_link_libraries(DarwinOperatingSystem PUBLIC OperatingSystem)
target_link_libraries(DarwinOperatingSystem PUBLIC Utilities)
target_link_libraries(DarwinOperatingSystem PUBLIC Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC DarwinOperatingSystem)

target_include_directories(DarwinOperatingSystem PRIVATE ${CMAKE_CURRENT_LIST_DIR})

if (ESP_PLATFORM)
  target_include_directories(DarwinOperatingSystem PRIVATE $<TARGET_PROPERTY:__idf_main,INTERFACE_INCLUDE_DIRECTORIES>)
  target_include_directories(__idf_main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
else()
  target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
endif()

target_compile_options(DarwinOperatingSystem PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
