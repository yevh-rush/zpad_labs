#pragma once

#include <opencv2/opencv.hpp>

class CameraProvider {
public:
    explicit CameraProvider(int deviceIndex = 0);
    bool isOpened() const;
    cv::Mat getFrame();

private:
    cv::VideoCapture capture_;
};
