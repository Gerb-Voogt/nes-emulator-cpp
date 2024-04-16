cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

project(nes-emu VERSION 0.1)

# Get all source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(nes-emu src/main.cpp ${SRC_FILES})
target_include_directories(nes-emu PUBLIC ${CMAKE_SOURCE_DIR}/include)
