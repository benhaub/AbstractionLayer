target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  NtpClient.hpp
)

add_library(NtpClient
OBJECT
  NtpClient.cpp
)

target_include_directories(NtpClient PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NtpClient PRIVATE abstractionLayer)
target_link_libraries(NtpClient PRIVATE IpClient)
target_link_libraries(NtpClient PRIVATE Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NtpClient)

target_compile_options(NtpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NtpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()