cmake_minimum_required(VERSION 3.12)
project(interference)

set(CMAKE_CXX_STANDARD 14)

add_library(interference
        src/neuron/neuron.cpp include/indk/neuron.h src/neuron/entry.cpp src/neuron/synapse.cpp src/neuron/receptor.cpp
        src/neuralnet/neuralnet.cpp include/indk/neuralnet.h
        src/error.cpp include/indk/error.h src/system.cpp include/indk/system.h src/position.cpp include/indk/position.h
        src/computer.cpp include/indk/computer.h
        src/backends/default.cpp include/indk/backends/default.h
        src/backends/multithread.cpp include/indk/backends/multithread.h
        src/backends/opencl.cpp include/indk/backends/opencl.h src/interlink.cpp include/indk/interlink.h)

include_directories(include/)
include_directories(3rdparty/)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads -latomic)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} -lws2_32)
ENDIF()

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
