cmake_minimum_required(VERSION 3.7)
project(cppcub3r)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

add_definitions(-DEV3DEV_PLATFORM=EV3)
add_subdirectory(ev3dev-lang-cpp)
add_subdirectory(kociemba)

add_subdirectory(src)
