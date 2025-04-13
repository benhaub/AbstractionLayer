target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  Nxppcf8506aModule.hpp
)

add_library(NxpPcf85063aNxppcf8506a
STATIC
  Nxppcf8506aModule.cpp
)
target_include_directories(Nxppcf8506a INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(NxpPcf85063aNxppcf8506a PUBLIC abstractionLayer)
target_link_libraries(NxpPcf85063aNxppcf8506a PUBLIC I2c)
target_link_libraries(NxpPcf85063aNxppcf8506a PUBLIC Logging)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC NxpPcf85063aNxppcf8506a)

target_include_directories(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(NxpPcf85063aNxppcf8506a PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)