cmake_minimum_required(VERSION 3.12)
project(interference)

set(CMAKE_CXX_STANDARD 14)

add_library(interference src/neuron/neuron.cpp include/inn/neuron.h
        src/neuron/entry.cpp src/neuron/synapse.cpp src/neuron/receptor.cpp src/position.cpp include/inn/position.h
        src/error.cpp include/inn/error.h src/neuralnet/neuralnet.cpp include/inn/neuralnet.h
        include/inn/system.h src/system.cpp
        src/computer.cpp include/inn/computer.h
        src/backends/default.cpp include/inn/backends/default.h
        src/backends/multithread.cpp include/inn/backends/multithread.h )

if(WIN32)
    #include_directories(interference "C:/boost/compute/include/")
    #include_directories(interference "C:/Program Files (x86)/AMD APP SDK/3.0/include")
    #target_link_libraries(interference "C:/Program Files (x86)/AMD APP SDK/3.0/lib/x86/OpenCL.lib")
    install(TARGETS interference ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)
else()
    install(TARGETS interference ARCHIVE DESTINATION lib)
    install(DIRECTORY include/ DESTINATION include)
endif()

add_subdirectory(test)
