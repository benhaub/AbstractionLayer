target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PowerResetClockManagementModule.hpp
)

add_library(NonePowerResetClockManagement
OBJECT
  PowerResetClockManagementModule.cpp
)

target_include_directories(PowerResetClockManagement INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NonePowerResetClockManagement PRIVATE abstractionLayer)
target_link_libraries(NonePowerResetClockManagement PRIVATE PowerResetClockManagement)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE NonePowerResetClockManagement)

target_compile_options(NonePowerResetClockManagement PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(NonePowerResetClockManagement PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE $<TARGET_PROPERTY:PowerResetClockManagement,INTERFACE_INCLUDE_DIRECTORIES>)
endif()