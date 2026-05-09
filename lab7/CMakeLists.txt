cmake_minimum_required(VERSION 3.20)
project(lab_07_face_detection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio dnn)
find_package(Threads REQUIRED)

add_executable(lab_07
    src/main.cpp
    src/camera_provider.cpp
    src/key_processor.cpp
    src/frame_processor.cpp
    src/display.cpp
    src/face_detector.cpp
)

target_include_directories(lab_07 PRIVATE include)
target_link_libraries(lab_07 PRIVATE ${OpenCV_LIBS} Threads::Threads)
