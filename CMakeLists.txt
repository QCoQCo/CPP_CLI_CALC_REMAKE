cmake_minimum_required(VERSION 3.10)
project(CLICalculator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(calculator main.cpp)

# Set output directory
set_target_properties(calculator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
