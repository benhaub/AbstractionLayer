target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PowerResetClockManagementModule.hpp
)

add_library(Cc32xxPowerResetClockManagement
OBJECT
  PowerResetClockManagementModule.cpp
)

target_include_directories(PowerResetClockManagement INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(Cc32xxPowerResetClockManagement PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(Cc32xxPowerResetClockManagement PRIVATE abstractionLayer)
target_link_libraries(Cc32xxPowerResetClockManagement PRIVATE PowerResetClockManagement)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxPowerResetClockManagement)

target_compile_options(Cc32xxPowerResetClockManagement PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxPowerResetClockManagement PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)