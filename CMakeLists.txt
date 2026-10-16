cmake_minimum_required(VERSION 3.16)

project(armordetector_final)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(include)

add_executable(armordetector_final
    src/main.cpp 
    src/video_capture.cpp
    src/color_detector.cpp    
)

target_link_libraries(armordetector_final ${OpenCV_LIBS})
