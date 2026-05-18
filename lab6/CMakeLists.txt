cmake_minimum_required(VERSION 3.20)
project(lab_06_opencv_camera LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio)

add_executable(lab_06
    src/main.cpp
    src/camera_provider.cpp
    src/key_processor.cpp
    src/frame_processor.cpp
    src/display.cpp
)

target_include_directories(lab_06 PRIVATE include)
target_link_libraries(lab_06 PRIVATE ${OpenCV_LIBS})
