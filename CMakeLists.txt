cmake_minimum_required(VERSION 3.12)
project(interference VERSION 2.0.0 DESCRIPTION "Interference library")

set(CMAKE_CXX_STANDARD 14)

set(TARGET_SOURCE
        src/neuron/neuron.cpp include/indk/neuron.h src/neuron/entry.cpp src/neuron/synapse.cpp src/neuron/receptor.cpp
        src/neuralnet/neuralnet.cpp include/indk/neuralnet.h
        src/error.cpp include/indk/error.h src/system.cpp include/indk/system.h src/position.cpp include/indk/position.h
        src/computer.cpp include/indk/computer.h
        src/backends/default.cpp include/indk/backends/default.h
        src/backends/multithread.cpp include/indk/backends/multithread.h
        src/backends/opencl.cpp include/indk/backends/opencl.h src/interlink.cpp include/indk/interlink.h
        src/profiler.cpp include/indk/profiler.h)

add_library(objlib OBJECT ${TARGET_SOURCE})
set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE ON)

function(build_indk LIBTYPE)
    message("Building ${LIBTYPE} variant")

    if(${LIBTYPE} STREQUAL shared)
        set(LIBTYPE "")
        add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:objlib>)
    elseif(${LIBTYPE} STREQUAL static)
        set(LIBTYPE "_static")
        add_library(${PROJECT_NAME}${LIBTYPE} STATIC $<TARGET_OBJECTS:objlib>)
    endif()

    set_target_properties(${PROJECT_NAME}${LIBTYPE} PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR})

    include_directories(include/)
    include_directories(3rdparty/)

    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME}${LIBTYPE} Threads::Threads -latomic)

    if (WIN32)
        target_link_libraries(${PROJECT_NAME}${LIBTYPE} -lws2_32)
    ENDIF()

    OPTION(INDK_OPENCL_SUPPORT "Interference NDK OpenCL compute backend support" OFF)

    if(INDK_OPENCL_SUPPORT)
        find_package(OpenCL REQUIRED)
        target_link_libraries(${PROJECT_NAME}${LIBTYPE} ${OpenCL_LIBRARIES})

        include_directories(${OpenCL_INCLUDE_DIRS})

        target_compile_definitions(${PROJECT_NAME}${LIBTYPE} PRIVATE CL_HPP_TARGET_OPENCL_VERSION=120)
        target_compile_definitions(${PROJECT_NAME}${LIBTYPE} PRIVATE CL_HPP_MINIMUM_OPENCL_VERSION=120)
        target_compile_definitions(${PROJECT_NAME}${LIBTYPE} PRIVATE CL_TARGET_OPENCL_VERSION=120)

        add_compile_definitions(INDK_OPENCL_SUPPORT)
        message(STATUS "Interference NDK OpenCL compute backend support is enabled")
    endif()
endfunction()

build_indk(shared)
build_indk(static)

if (WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/dist/lib NAMELINK_COMPONENT)
else()
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/dist/lib NAMELINK_COMPONENT)
endif()

install(TARGETS ${PROJECT_NAME}_static ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/dist/lib NAMELINK_COMPONENT)
install(DIRECTORY include DESTINATION ${CMAKE_SOURCE_DIR}/dist)

add_subdirectory(samples/test)
add_subdirectory(samples/vision)
add_subdirectory(samples/multimodal)
add_subdirectory(samples/lang)
