cmake_minimum_required(VERSION 3.15)
project(MyEmulatorDap CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(emulator)
add_subdirectory(adapter)
