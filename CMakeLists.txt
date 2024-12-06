cmake_minimum_required(VERSION 3.28)
project(L2SB_LIB)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-exceptions -march=native -funroll-loops -mtune=native" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -march=native -mtune=native" CACHE STRING "" FORCE)
add_executable(L2SB_LIB
        src/l2sb.hpp
        src/test.cpp)
add_executable(Real
        src/ML2SB.hpp
        src/ML2SB.cpp
        src/Genetics.cpp
        src/Genetics.hpp
        src/Gene.cpp
        src/Gene.hpp)
target_compile_options(Real PUBLIC -Wall -Wextra -Wpedantic)