cmake_minimum_required(VERSION 3.10)
project(CSVParser)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
option(ARIA_CSV_ENABLE_PROPERTY_TESTS "Build RapidCheck property tests" OFF)

# Find or fetch GTest
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.12.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(parser_test parser_test.cpp)
target_compile_features(parser_test PRIVATE cxx_std_11)
target_link_libraries(parser_test PRIVATE gtest_main)

if(ARIA_CSV_ENABLE_PROPERTY_TESTS)
  set(RC_ENABLE_GTEST OFF CACHE BOOL "" FORCE)
  set(RC_ENABLE_GMOCK OFF CACHE BOOL "" FORCE)
  set(RC_ENABLE_BOOST_TEST OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG b2d9ed2dddefc4b84318d664b4f221eb792d89c7
  )
  FetchContent_MakeAvailable(rapidcheck)

  add_executable(parser_property_test property_test.cpp)
  target_compile_features(parser_property_test PRIVATE cxx_std_11)
  target_link_libraries(parser_property_test PRIVATE rapidcheck)
endif()
