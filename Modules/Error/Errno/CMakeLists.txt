add_library(ErrnoError
OBJECT
  ErrorModule.cpp
)

target_link_libraries(ErrnoError PRIVATE Utilities)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ErrnoError)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(ErrnoError PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(ErrnoError PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
