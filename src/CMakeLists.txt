set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED COMPONENTS log)
find_package(OpenCV REQUIRED COMPONENTS core videoio imgproc imgcodecs)

include_directories(../kociemba/src)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

add_ev3_executable(cppcub3r main.cpp)

target_link_libraries(cppcub3r ${Boost_LIBRARIES})

add_ev3_executable(detection detection.cpp)

target_link_libraries(detection Kociemba)
target_link_libraries(detection ${Boost_LIBRARIES})
target_link_libraries(detection ${OpenCV_LIBRARIES})

add_ev3_executable(picture picture.cpp)

target_link_libraries(picture ${Boost_LIBRARIES})
target_link_libraries(picture ${OpenCV_LIBRARIES})


