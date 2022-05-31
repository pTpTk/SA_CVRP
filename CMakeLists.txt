cmake_minimum_required(VERSION 3.15)

project(HGS-CVRP-SP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-g -O3")
#set(CMAKE_CXX_COMPILER "/software/gcc/9.1.0/bin/g++")
#set(CMAKE_C_COMPILER "/software/gcc/9.1.0/bin/gcc")

# include(FetchContent)
# FetchContent_Declare(
#   or-tools
#   GIT_REPOSITORY https://github.com/google/or-tools.git
#   GIT_TAG        master
# )
# FetchContent_MakeAvailable(or-tools)

file(GLOB SRC
    src/*.cpp
)

file(GLOB HEADER
    include/*.h
)

include_directories(include)

add_library(cvrplib ${SRC} ${HEADER})

add_executable(cvrp main/main.cpp)

# target_link_libraries(cvrplib ortools::ortools)
target_link_libraries(cvrp cvrplib)
