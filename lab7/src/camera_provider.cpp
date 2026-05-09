#include "camera_provider.hpp"

#include <iostream>

CameraProvider::CameraProvider(int deviceIndex) {
    if (!capture_.open(deviceIndex, cv::CAP_V4L2)) {
        capture_.open(deviceIndex);
    }

    if (capture_.isOpened()) {
        capture_.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        capture_.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
        capture_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    }
}

bool CameraProvider::isOpened() const {
    return capture_.isOpened();
}

cv::Mat CameraProvider::getFrame() {
    cv::Mat frame;
    if (capture_.isOpened()) {
        capture_ >> frame;
    }
    return frame;
}
