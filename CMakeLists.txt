cmake_minimum_required(VERSION 3.30)
project(smp_dev)

set(CMAKE_CXX_STANDARD 17)

add_executable(smp_dev main.cpp
        src/utils/Random.hpp
        src/utils/Random.cpp
        src/utils/Random.hpp)
