target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  CryptographyModule.hpp
)

add_library(EspCryptography
OBJECT
  CryptographyModule.cpp
)

target_include_directories(Cryptography INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:Cryptography,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(EspCryptography PRIVATE abstractionLayer)
target_link_libraries(EspCryptography PRIVATE Cryptography)
target_link_libraries(EspCryptography PRIVATE Storage)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE EspCryptography)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(EspCryptography PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(EspCryptography PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)