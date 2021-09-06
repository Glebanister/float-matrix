cmake_minimum_required(VERSION 3.15)
project(float-matrix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow")

add_subdirectory(extern/compute)

set(TEST test-${CMAKE_PROJECT_NAME})
set(TEST_SRCS
    test/test.cpp
    test/overall.cpp
    test/stress.cpp
)

add_executable(${TEST} ${TEST_SRCS})
target_include_directories(${TEST} PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(${TEST} PUBLIC extern/doctest/doctest)
target_include_directories(${TEST} PUBLIC src)
target_link_libraries(${TEST} ${OpenCL_LIBRARY})
