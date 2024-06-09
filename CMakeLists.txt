cmake_minimum_required(VERSION 3.25)
project(NESEmulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(NESEmulator main.cpp state.cpp basics.h instructions.cpp cpu.cpp instructions.h state.h)
