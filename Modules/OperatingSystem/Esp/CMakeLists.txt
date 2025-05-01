target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  OperatingSystemModule.hpp
)

add_library(EspOperatingSystem
OBJECT
  OperatingSystemModule.cpp
)

target_include_directories(OperatingSystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(OperatingSystem INTERFACE $<TARGET_PROPERTY:__idf_main,INCLUDE_DIRECTORIES>)
target_include_directories(OperatingSystem INTERFACE $<TARGET_PROPERTY:Utilities,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:OperatingSystem,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(EspOperatingSystem PRIVATE abstractionLayer)
target_link_libraries(EspOperatingSystem PRIVATE OperatingSystem)
target_link_libraries(EspOperatingSystem PRIVATE Processor)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspOperatingSystem)

target_compile_options(EspOperatingSystem PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspOperatingSystem PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
