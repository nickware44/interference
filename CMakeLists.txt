cmake_minimum_required(VERSION 3.12)
project(Interference)

set(CMAKE_CXX_STANDARD 17)

add_library(Interference src/neuron/neuron.cpp include/inn/neuron.h src/neuron/entry.cpp src/neuron/system.cpp src/neuron/synaps.cpp src/neuron/receptor.cpp src/position.cpp include/inn/position.h src/error.cpp include/inn/error.h src/neuralnet/neuralnet.cpp include/inn/neuralnet.h src/neuralnet/link.cpp)