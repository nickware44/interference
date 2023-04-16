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

IF (UNIX)
    add_compile_definitions(INN_OPENCL_SUPPORT)
    target_link_libraries(${PROJECT_NAME} "/usr/lib/x86_64-linux-gnu/libOpenCL.so")
    include_directories(/usr/include/CL/)
ELSE()
    add_compile_definitions(INN_OPENCL_SUPPORT)
    target_link_libraries(${PROJECT_NAME} "D:/Workspace/CUDA/lib/x64/OpenCL.lib")
    include_directories(D:/Workspace/CUDA/include)
ENDIF()

install(TARGETS interference ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(samples/test)
add_subdirectory(samples/vision)
add_subdirectory(samples/multimodal)
