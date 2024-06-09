cmake_minimum_required(VERSION 3.25)
project(NESEmulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(NESEmulator src/core/main.cpp src/core/state.cpp src/core/basics.h src/core/instructions.cpp src/core/cpu.cpp src/core/instructions.h src/core/state.h)

# Find the nlohmann-json package
find_package(nlohmann_json 3.2.0 REQUIRED)


# Link the nlohmann-json library
target_link_libraries(NESEmulator PRIVATE nlohmann_json::nlohmann_json)
