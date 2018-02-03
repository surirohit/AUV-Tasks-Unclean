cmake_minimum_required(VERSION 2.8)

project(AUV_TASKS)

find_package( OpenCV REQUIRED )

add_executable( gate src/gate.cpp )
target_link_libraries( gate ${OpenCV_LIBS} )

add_executable( split src/split.cpp )
target_link_libraries( split ${OpenCV_LIBS} )
