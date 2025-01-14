target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  Log.hpp
)

target_include_directories(Logging INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Logging INTERFACE Event)

target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC Logging)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if (ESP_PLATFORM)
  target_include_directories(__idf_main PUBLIC $<TARGET_PROPERTY:Logging,INTERFACE_INCLUDE_DIRECTORIES>)
endif()