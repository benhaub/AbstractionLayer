target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  OperatingSystemModule.hpp
)

add_library(DarwinOperatingSystem
OBJECT
  OperatingSystemModule.cpp
)

target_include_directories(OperatingSystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(DarwinOperatingSystem PRIVATE abstractionLayer)
target_link_libraries(DarwinOperatingSystem PRIVATE OperatingSystem)
target_link_libraries(DarwinOperatingSystem PRIVATE Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE DarwinOperatingSystem)

target_include_directories(DarwinOperatingSystem PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(DarwinOperatingSystem PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
