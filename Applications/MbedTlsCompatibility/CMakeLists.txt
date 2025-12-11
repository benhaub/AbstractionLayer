target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  MbedTlsSendReceive.hpp
)

add_library(MbedTlsSendReceive
OBJECT
  MbedTlsSendReceive.cpp
)

target_include_directories(MbedTlsSendReceive PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(abstractionLayer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(MbedTlsSendReceive PRIVATE Utilities)
target_link_libraries(MbedTlsSendReceive PRIVATE Network)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE MbedTlsSendReceive)

target_compile_options(MbedTlsSendReceive PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(MbedTlsSendReceive PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
else()
  #Espressif has an mbedtls component.
  target_link_libraries(MbedTlsSendReceive PUBLIC MbedTls)
endif()