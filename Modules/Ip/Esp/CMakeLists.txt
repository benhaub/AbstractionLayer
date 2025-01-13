target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  IpClientModule.hpp
)
#Client
add_library(EspIp
STATIC
  IpClientModule.cpp
)

target_include_directories(IpClient INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(EspIp PUBLIC abstractionLayer)
target_link_libraries(EspIp PUBLIC IpClient)
target_link_libraries(EspIp PUBLIC Logging)
target_link_libraries(EspIp PUBLIC Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC EspIp)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(EspIp PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
