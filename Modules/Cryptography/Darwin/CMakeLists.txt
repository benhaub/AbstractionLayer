target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  CryptographyModule.hpp
)

add_library(DarwinCryptography
STATIC
  CryptographyModule.cpp
)
target_include_directories(Cryptography INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_include_directories(DarwinCryptography PRIVATE ${CMAKE_CURRENT_LIST_DIR}/libsodium-stable/src/libsodium/include)

target_link_libraries(DarwinCryptography PUBLIC Utilities)
target_link_libraries(DarwinCryptography PUBLIC abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC DarwinCryptography)

set(libsodium "libsodium-1.0.20-stable")
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_LIST_DIR}/${libsodium}.tar.gz
    COMMAND
    rm -f ${libsodium}.tar.gz
  COMMAND
    wget https://download.libsodium.org/libsodium/releases/${libsodium}.tar.gz
  COMMAND
    tar -xf ${libsodium}.tar.gz
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}
  COMMENT
    "Downloading ${libsodium}..."
  VERBATIM
)
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_LIST_DIR}/libsodium-stable/src/libsodium/.libs/libsodium.a
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}/libsodium-stable
  COMMENT
    "Building ${libsodium}..."
  COMMAND
    ./configure && make && make check
  VERBATIM
)
add_custom_target(libsodium-stable_download DEPENDS ${libsodium}.tar.gz)
add_custom_target(libsodium-stable DEPENDS ${CMAKE_CURRENT_LIST_DIR}/libsodium-stable/src/libsodium/.libs/libsodium.a)
add_dependencies(libsodium-stable libsodium-stable_download)
add_dependencies(DarwinCryptography libsodium-stable)

target_link_libraries(DarwinCryptography PUBLIC ${CMAKE_CURRENT_LIST_DIR}/libsodium-stable/src/libsodium/.libs/libsodium.a)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(DarwinCryptography PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(DarwinCryptography PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)