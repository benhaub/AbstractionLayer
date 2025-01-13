target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PowerResetClockManagementModule.hpp
)

add_library(NonePowerResetClockManagement
STATIC
  PowerResetClockManagementModule.cpp
)
target_include_directories(PowerResetClockManagement INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NonePowerResetClockManagement PUBLIC Utilities)
target_link_libraries(NonePowerResetClockManagement PUBLIC abstractionLayer)

target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NonePowerResetClockManagement)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NonePowerResetClockManagement PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)