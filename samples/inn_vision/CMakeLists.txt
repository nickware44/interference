cmake_minimum_required(VERSION 3.12)
project(inn_vision_sample)
set(CMAKE_CXX_STANDARD 17)

add_executable(inn_vision_sample main.cpp lodepng.cpp)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    set(INTERFERENCE_ROOT "../..")
    include_directories("${INTERFERENCE_ROOT}/include")
    find_library(INTERFERENCE_LIB "${INTERFERENCE_ROOT}/lib/libinterference.a")
    target_link_libraries(${PROJECT_NAME} ${INTERFERENCE_LIB})
else()
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} interference Threads::Threads -latomic)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /bin)
