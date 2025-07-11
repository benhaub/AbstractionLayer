target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientModule.hpp
  IpServerModule.hpp
  HttpServerModule.hpp
  HttpClientModule.hpp
  HttpsClientModule.hpp
)
#Client
add_library(PosixIpClient
OBJECT
  IpClientModule.cpp
  HttpClientModule.cpp
  HttpsClientModule.cpp
)

target_link_libraries(PosixIpClient PRIVATE abstractionLayer)
target_link_libraries(PosixIpClient PRIVATE Network)
target_link_libraries(PosixIpClient PRIVATE OperatingSystem)
target_link_libraries(PosixIpClient PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE PosixIpClient)

set(MBEDTLS_BUILD_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/mbedtls/build_mbedtls.sh")
file(WRITE "${MBEDTLS_BUILD_SCRIPT}" "
#!/bin/bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r scripts/basic.requirements.txt
mkdir -p mbedtls_Build
cd mbedtls_Build
cmake -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF -DUSE_SHARED_MBEDTLS_LIBRARY=OFF -DUSE_STATIC_MBEDTLS_LIBRARY=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
deactivate
")
execute_process(
  COMMAND
    chmod +x "${MBEDTLS_BUILD_SCRIPT}"
  COMMAND
    bash "${MBEDTLS_BUILD_SCRIPT}"
  WORKING_DIRECTORY
    "${CMAKE_CURRENT_LIST_DIR}/mbedtls"
  OUTPUT_QUIET
  COMMAND_ERROR_IS_FATAL
    ANY
)
file(REMOVE "${MBEDTLS_BUILD_SCRIPT}")

target_include_directories(PosixIpClient
PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/mbedtls/include
  ${CMAKE_CURRENT_LIST_DIR}/mbedtls/tf-psa-crypto/include
)

set(MbedTLS_DIR ${CMAKE_CURRENT_LIST_DIR}/mbedtls/mbedtls_Build/cmake)
find_package(MbedTLS)
target_link_libraries(PosixIpClient
PUBLIC
  MbedTLS::mbedtls
  MbedTLS::mbedx509
)

target_compile_options(PosixIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(PosixIpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

#Server
add_library(PosixIpServer
OBJECT
  IpServerModule.cpp
  HttpServerModule.cpp
)

target_link_libraries(PosixIpServer PRIVATE abstractionLayer)
target_link_libraries(PosixIpServer PRIVATE Network)
target_link_libraries(PosixIpServer PRIVATE OperatingSystem)
target_link_libraries(PosixIpServer PRIVATE Event)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE PosixIpServer)

target_compile_options(PosixIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(PosixIpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
