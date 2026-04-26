#include "camera_provider.hpp"

CameraProvider::CameraProvider(int deviceIndex) {
    capture_.open(deviceIndex);
    if (capture_.isOpened()) {
        capture_.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        capture_.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
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
