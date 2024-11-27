# Minimum required CMake version
cmake_minimum_required(VERSION 3.15)

# Project name and version
project(CircularBufferProject VERSION 1.0)

# Specify the C standard (MISRA recommends C99 or later)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
        test/test_temon.c
        src/temperature_monitor.c
)

# Add the include directory
include_directories(src)

# Create an executable target for the main program
add_executable(test_temon ${SOURCES})

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

# Add a custom target to run tests with `make test`
add_custom_target(tests
        COMMAND run_tests
        DEPENDS run_tests
        COMMENT "Running tests"
)
