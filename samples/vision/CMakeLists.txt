cmake_minimum_required(VERSION 3.12)
project(interference_vision)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} main.cpp bmp.hpp)

include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} interference_static)

set(SAMPLE_INSTALL_DIR ${CMAKE_SOURCE_DIR}/dist/bin/vision)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${SAMPLE_INSTALL_DIR})
install(DIRECTORY images DESTINATION ${SAMPLE_INSTALL_DIR})
install(DIRECTORY structures DESTINATION ${SAMPLE_INSTALL_DIR})
