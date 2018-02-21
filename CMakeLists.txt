cmake_minimum_required(VERSION 2.8)

project(AUV_TASKS)

find_package( OpenCV 3.2 REQUIRED )

include_directories(include/)

add_library(GateEdge include/GateEdge.h src/GateEdge.cpp)
target_link_libraries( GateEdge ${OpenCV_LIBS} )

add_library(GateFinal include/GateFinal.h src/GateFinal.cpp)
target_link_libraries( GateFinal ${OpenCV_LIBS} )

add_executable(test src/main.cpp)
target_link_libraries(test GateEdge GateFinal ${OpenCV_LIBS})

add_executable(test1 src/test.cpp)
target_link_libraries(test1 ${OpenCV_LIBS})

add_executable(split src/split.cpp)
target_link_libraries(split ${OpenCV_LIBS})

add_executable(yaw src/yaw.cpp)
target_link_libraries(yaw ${OpenCV_LIBS})
