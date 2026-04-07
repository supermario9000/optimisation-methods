cmake_minimum_required(VERSION 3.10)
project(OptimizationMethods)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics system window REQUIRED)

# Add executable
add_executable(optimization_methods main.cpp algoritmai.cpp)

# Link SFML
target_link_libraries(optimization_methods PRIVATE sfml-graphics sfml-system sfml-window)
