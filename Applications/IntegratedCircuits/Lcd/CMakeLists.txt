target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  LcdAbstraction.hpp
  Riverdi/Eve3Tft3_5Inch.hpp
  MatrixOrbital/Eve3Tft3_5Inch.hpp
  Bridgetek/Bridgetek81x.hpp
  Bridgetek/Bridgetek81xTypes.hpp
)

target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE
  Bridgetek/Bridgetek81x.cpp
)

add_library(RiverdiEve3Tft3_5Inch
OBJECT
  Riverdi/Eve3Tft3_5Inch.cpp
)

add_library(MatrixOrbitalEve3Tft3_5Inch
OBJECT
  MatrixOrbital/Eve3Tft3_5Inch.cpp
)

target_link_libraries(RiverdiEve3Tft3_5Inch PRIVATE abstractionLayer)
target_link_libraries(MatrixOrbitalEve3Tft3_5Inch PRIVATE abstractionLayer)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE RiverdiEve3Tft3_5Inch)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE MatrixOrbitalEve3Tft3_5Inch)

target_include_directories(RiverdiEve3Tft3_5Inch PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)
target_include_directories(MatrixOrbitalEve3Tft3_5Inch PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(RiverdiEve3Tft3_5Inch PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(RiverdiEve3Tft3_5Inch PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)
target_compile_options(MatrixOrbitalEve3Tft3_5Inch PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(MatrixOrbitalEve3Tft3_5Inch PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

if (ESP_PLATFORM)
  target_include_directories(__idf_main PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endif()