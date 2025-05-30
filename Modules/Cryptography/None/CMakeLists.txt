target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  CryptographyModule.hpp
)

add_library(NoneCryptography
OBJECT
  CryptographyModule.cpp
)

target_include_directories(Cryptography INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NoneCryptography PRIVATE abstractionLayer)
target_link_libraries(NoneCryptography PRIVATE Cryptography)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NoneCryptography)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NoneCryptography PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NoneCryptography PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Cryptography,INTERFACE_INCLUDE_DIRECTORIES>)
endif()