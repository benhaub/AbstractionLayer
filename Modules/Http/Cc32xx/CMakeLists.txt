target_sources(${PROJECT_NAME}${EXECUTABLE_SUFFIX}
PRIVATE FILE_SET headers TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES
  HttpServerModule.hpp
  HttpClientModule.hpp
  HttpsClientModule.hpp
)

#Client
add_library(Cc32xxHttpClient
OBJECT
  HttpClientModule.cpp
  HttpsClientModule.cpp
)

target_include_directories(Http INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Cc32xxHttpClient PRIVATE abstractionLayer)
target_include_directories(Cc32xxHttpClient PRIVATE Http)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxHttpClient)

target_include_directories(Cc32xxHttpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Cc32xxHttpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxHttpClient PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)

#Server
add_library(Cc32xxHttpServer
OBJECT
  HttpServerModule.cpp
)

target_link_libraries(Cc32xxHttpServer PRIVATE abstractionLayer)
target_link_libraries(Cc32xxHttpServer PRIVATE Http)
target_link_libraries(${PROJECT_NAME}${EXECUTABLE_SUFFIX} PRIVATE Cc32xxHttpServer)

target_include_directories(Cc32xxHttpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},INCLUDE_DIRECTORIES>)

target_compile_options(Cc32xxHttpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_OPTIONS>)
target_compile_definitions(Cc32xxHttpServer PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME}${EXECUTABLE_SUFFIX},COMPILE_DEFINITIONS>)