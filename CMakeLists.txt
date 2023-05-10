cmake_minimum_required(VERSION 3.12)
project(interference)

set(CMAKE_CXX_STANDARD 14)

add_library(interference
        src/neuron/neuron.cpp include/inn/neuron.h src/neuron/entry.cpp src/neuron/synapse.cpp src/neuron/receptor.cpp
        src/neuralnet/neuralnet.cpp include/inn/neuralnet.h
        src/error.cpp include/inn/error.h src/system.cpp include/inn/system.h src/position.cpp include/inn/position.h
        src/computer.cpp include/inn/computer.h
        src/backends/default.cpp include/inn/backends/default.h
        src/backends/multithread.cpp include/inn/backends/multithread.h
        src/backends/opencl.cpp include/inn/backends/opencl.h)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads -latomic)

OPTION(INDK_OPENCL_SUPPORT "Interference NDK OpenCL compute backend support" OFF)

if(INDK_OPENCL_SUPPORT)
    find_package(OpenCL REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${OpenCL_LIBRARIES})
    include_directories(${OpenCL_INCLUDE_DIRS})

    target_compile_definitions(${PROJECT_NAME} PRIVATE CL_HPP_TARGET_OPENCL_VERSION=120)
    target_compile_definitions(${PROJECT_NAME} PRIVATE CL_HPP_MINIMUM_OPENCL_VERSION=120)
    target_compile_definitions(${PROJECT_NAME} PRIVATE CL_TARGET_OPENCL_VERSION=120)

    add_compile_definitions(INDK_OPENCL_SUPPORT)
    message(STATUS "Interference NDK OpenCL compute backend support is enabled")
ENDIF()

install(TARGETS interference ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/dist/lib)
install(DIRECTORY include DESTINATION ${CMAKE_SOURCE_DIR}/dist/)

add_subdirectory(samples/test)
add_subdirectory(samples/vision)
add_subdirectory(samples/multimodal)
