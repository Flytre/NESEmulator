cmake_minimum_required(VERSION 3.25)
project(NESEmulator)

set(CMAKE_CXX_STANDARD 17)

add_executable(NESEmulator main.cpp datapath.cpp state.cpp)
