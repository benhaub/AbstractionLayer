target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  OperatingSystemModule.hpp
)

add_library(cc32xxOperatingSystem
STATIC
  OperatingSystemModule.cpp
)

target_include_directories(OperatingSystem INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(cc32xxOperatingSystem PUBLIC abstractionLayer)
target_link_libraries(cc32xxOperatingSystem PUBLIC OperatingSystem)
target_link_libraries(cc32xxOperatingSystem PUBLIC Utilities)
target_link_libraries(cc32xxOperatingSystem PUBLIC Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC cc32xxOperatingSystem)

target_include_directories(cc32xxOperatingSystem PRIVATE ${CMAKE_CURRENT_LIST_DIR})

#FreeRTOS includes will not work unless the application code helps us out by supplying the headers.
#We do not have any knowledge of where the application is getting there FreeRTOS headers from.
#The application will need to add a line into their CMakeFiles.txt along the lines of:
#target_include_directories(OperatingSystem INTERFACE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INTERFACE_INCLUDE_DIRECTORIES>)
#So that this modules is aware of where the FreeRTOS headers can be found.

if (ESP_PLATFORM)
  target_include_directories(cc32xxOperatingSystem PRIVATE $<TARGET_PROPERTY:__idf_main,INTERFACE_INCLUDE_DIRECTORIES>)
  target_include_directories(__idf_main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
else()
  target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
endif()

target_compile_options(cc32xxOperatingSystem PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
