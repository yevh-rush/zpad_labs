#include "display.hpp"

Display::Display(const std::string& windowName) : windowName_(windowName) {}

void Display::show(const cv::Mat& frame) const {
    cv::imshow(windowName_, frame);
}

const std::string& Display::windowName() const {
    return windowName_;
}
