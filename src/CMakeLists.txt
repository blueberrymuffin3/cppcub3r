set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED COMPONENTS log)
find_package(OpenCV REQUIRED COMPONENTS core videoio imgproc imgcodecs)

include_directories(../kociemba/src)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(cppcub3r detection.cpp main.cpp motors.cpp solver.cpp)

target_link_libraries(cppcub3r ev3dev)
target_link_libraries(cppcub3r Kociemba)
target_link_libraries(cppcub3r ${Boost_LIBRARIES})
target_link_libraries(cppcub3r ${OpenCV_LIBRARIES})

add_ev3_executable(picture picture.cpp)

target_link_libraries(picture ${Boost_LIBRARIES})
target_link_libraries(picture ${OpenCV_LIBRARIES})
