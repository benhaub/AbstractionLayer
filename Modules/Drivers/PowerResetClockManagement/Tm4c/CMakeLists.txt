target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  PowerResetClockManagementModule.hpp
)

add_library(Tm4cPowerResetClockManagement
OBJECT
  PowerResetClockManagementModule.cpp
)

target_include_directories(PowerResetClockManagement INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(PowerResetClockManagement INTERFACE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_link_libraries(Tm4cPowerResetClockManagement PRIVATE abstractionLayer)
target_link_libraries(Tm4cPowerResetClockManagement PRIVATE PowerResetClockManagement)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Tm4cPowerResetClockManagement)

target_compile_options(Tm4cPowerResetClockManagement PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Tm4cPowerResetClockManagement PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)