cmake_minimum_required(VERSION 3.20)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE
    "Debug"
    CACHE STRING "Default build type")
set(CMAKE_CXX_FLAGS_DEBUG
    "-Og -ggdb3 -fverbose-asm -Wall -Wextra"
    CACHE STRING "Debug Configuration Flags")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
project(Compression VERSION 1.0.0)

list(APPEND sources ./bitstream.cpp ./huffman.cpp)
add_executable(Compression ./main.cpp "${sources}")

set(IS_TESTING_ON ON)
if(${IS_TESTING_ON})
  # include(FetchContent) FetchContent_Declare(googletest GITHUB
  # https://github.com/google/googletest.git)
  # FetchContent_MakeAvailable(googletest)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
  enable_testing()

  include(GoogleTest)
  find_package(GTest)
  list(APPEND unit_tests ibitstream obitstream huffman)

  foreach(unit_test IN LISTS unit_tests)
    add_executable(${unit_test} tests/${unit_test}.cpp "${sources}")

    target_include_directories(${unit_test} PUBLIC ${CMAKE_SOURCE_DIR})
    target_link_libraries(${unit_test} GTest::gtest_main)

    gtest_discover_tests(${unit_test})
  endforeach()
endif()
