cmake_minimum_required(VERSION 3.7)

add_definitions(-DEV3DEV_PLATFORM=EV3)
add_subdirectory(ev3dev-lang-cpp)
add_subdirectory(kociemba)

add_subdirectory(src)
