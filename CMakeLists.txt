# Minimum required CMake version
cmake_minimum_required(VERSION 3.15)

# Project name and version
project(CircularBufferProject VERSION 1.0)

# Use native GNU toolchain
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# Specify the C standard (C11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
        src/main.c
        src/temperature_monitor.c
)

# Add the include directory
include_directories(src)

# Create an executable target for the main program
add_executable(temperature_monitor ${SOURCES})

# Link required libraries
# For C11 threads on GCC, we need to link the threads library
find_package(Threads REQUIRED)
target_link_libraries(temperature_monitor Threads::Threads)

# Add the Unity testing executable
set(TEST_SOURCES
        test/test_temon.c
        src/temperature_monitor.c
        test/unity.c
)

# Add the include directories for the test target
include_directories(test)

# Create an executable target for the Unity tests
add_executable(run_tests ${TEST_SOURCES})

# Link Threads library for test executable
target_link_libraries(run_tests Threads::Threads)

# Add a custom target to run tests with `make test`
add_custom_target(tests
        COMMAND run_tests
        DEPENDS run_tests
        COMMENT "Running tests"
)