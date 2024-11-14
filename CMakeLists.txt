cmake_minimum_required(VERSION 3.29)
project(L2SB_LIB)

set(CMAKE_CXX_STANDARD 20)

include_directories(TOOLS)

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -Wall -Wpedantic")
add_executable(L2SB_LIB
    TOOLS/l2sb.hpp
    TOOLS/test.cpp)
