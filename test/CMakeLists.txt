cmake_minimum_required(VERSION 3.10)
project(CSVParser)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

# Find or fetch GTest
cmake_policy(SET CMP0135 NEW)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.12.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(parser_test parser_test.cpp)
target_link_libraries(parser_test PRIVATE gtest_main)
