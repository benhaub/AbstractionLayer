#Espressif has an mbedtls component.
if (NOT ESP_PLATFORM)

  #Set when you give a toolchain file to cmake.
  if(CMAKE_TOOLCHAIN_FILE)
    set(TOOLCHAIN_FILE_ARG "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
  endif()

  set(MBEDTLS_BUILD_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/mbedtls/build_mbedtls.sh")
  file(WRITE "${MBEDTLS_BUILD_SCRIPT}" "
  #!/bin/bash
  python3 -m venv .venv
  source .venv/bin/activate
  pip install -r scripts/basic.requirements.txt
  python scripts/config.py unset MBEDTLS_NET_C
  mkdir -p mbedtls_Build
  cd mbedtls_Build
  cmake ${TOOLCHAIN_FILE_ARG} -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF -DUSE_SHARED_MBEDTLS_LIBRARY=OFF -DUSE_STATIC_MBEDTLS_LIBRARY=ON -DCMAKE_BUILD_TYPE=Release ..
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

  add_library(MbedTls INTERFACE)

  target_include_directories(MbedTls
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/mbedtls/include
    ${CMAKE_CURRENT_LIST_DIR}/mbedtls/tf-psa-crypto/include
  )

  set(MbedTLS_DIR ${CMAKE_CURRENT_LIST_DIR}/mbedtls/mbedtls_Build/cmake)
  find_package(MbedTLS)
  target_link_libraries(MbedTls
  INTERFACE
    MbedTLS::tfpsacrypto
    MbedTLS::mbedtls
    MbedTLS::mbedx509
  )
endif()