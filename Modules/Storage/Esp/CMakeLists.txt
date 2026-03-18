target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  StorageModule.hpp
  FileSystemModule.hpp
)

add_library(EspStorage
OBJECT
  StorageModule.cpp
  FileSystemModule.cpp
)

target_include_directories(Storage INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Storage INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)
target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Storage,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(EspStorage PRIVATE abstractionLayer)
target_link_libraries(EspStorage PRIVATE Storage)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspStorage)

target_compile_options(EspStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspStorage PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
