#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class FaceDetector {
public:
    FaceDetector(const std::string& prototxtPath, const std::string& modelPath);

    bool isReady() const;
    std::vector<cv::Rect> detect(const cv::Mat& frame);

private:
    std::vector<cv::Rect> parseDetections(const cv::Mat& frame, const cv::Mat& detections) const;

    cv::dnn::Net net_;
    bool ready_;
};
