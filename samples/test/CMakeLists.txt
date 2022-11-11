cmake_minimum_required(VERSION 3.12)
project(interference_test)

set(CMAKE_CXX_STANDARD 14)

add_executable(interference_test main.cpp)

include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} interference)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
