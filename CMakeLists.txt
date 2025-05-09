cmake_minimum_required(VERSION 3.28)
project(L2SB_LIB)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic -fsanitize=undefined" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -march=native -mtune=native" CACHE STRING "" FORCE)
add_executable(L2SB_LIB
        src/Old/l2sb.hpp
        src/Old/test.cpp)
add_executable(Real
        src/Current/FitnessFunction.hpp
        src/Current/main.cpp
        src/Current/Types.hpp
        src/Current/Utils.hpp
        src/Current/FileData.cpp
        src/Current/FileData.hpp
        src/Current/Genetics.cpp
        src/Current/Genetics.hpp
        src/Current/BinString.hpp
        src/Current/BinString.cpp
        src/Current/StackVector.hpp
)
add_executable(Tests
        src/Current/FitnessFunction.hpp
        src/Current/tests.cpp
        src/Current/Types.hpp
        src/Current/Utils.hpp
        src/Current/FileData.cpp
        src/Current/FileData.hpp
        src/Current/Genetics.cpp
        src/Current/Genetics.hpp
        src/Current/BinString.hpp
        src/Current/BinString.cpp
        src/Current/StackVector.hpp
)
target_compile_options(Real PUBLIC -Wall -Wextra -Wpedantic)
#target_link_options(Real PUBLIC -flto=auto)