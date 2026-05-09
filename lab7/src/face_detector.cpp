#include "face_detector.hpp"

#include <algorithm>
#include <iostream>

FaceDetector::FaceDetector(const std::string& prototxtPath, const std::string& modelPath)
    : ready_(false) {
    try {
        net_ = cv::dnn::readNetFromCaffe(prototxtPath, modelPath);
        ready_ = true;
    } catch (const cv::Exception& e) {
        std::cerr << "Failed to load face detector: " << e.what() << std::endl;
    }
}

bool FaceDetector::isReady() const {
    return ready_;
}

std::vector<cv::Rect> FaceDetector::detect(const cv::Mat& frame) {
    if (!ready_ || frame.empty()) {
        return {};
    }

    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300),
                                          cv::Scalar(104.0, 177.0, 123.0));
    net_.setInput(blob);
    cv::Mat detections = net_.forward();

    return parseDetections(frame, detections);
}

std::vector<cv::Rect> FaceDetector::parseDetections(const cv::Mat& frame, const cv::Mat& detections) const {
    std::vector<cv::Rect> result;
    if (detections.empty() || detections.dims != 4 || detections.size[2] <= 0) {
        return result;
    }

    const int height = frame.rows;
    const int width = frame.cols;
    const float* data = reinterpret_cast<const float*>(detections.data);
    const int numDetections = detections.size[2];

    for (int i = 0; i < numDetections; ++i) {
        const float confidence = data[i * 7 + 2];
        if (confidence <= 0.5f) {
            continue;
        }

        const int x1 = std::clamp(static_cast<int>(data[i * 7 + 3] * width), 0, width - 1);
        const int y1 = std::clamp(static_cast<int>(data[i * 7 + 4] * height), 0, height - 1);
        const int x2 = std::clamp(static_cast<int>(data[i * 7 + 5] * width), 0, width - 1);
        const int y2 = std::clamp(static_cast<int>(data[i * 7 + 6] * height), 0, height - 1);

        const cv::Rect box(cv::Point(x1, y1), cv::Point(x2, y2));
        if (box.width > 0 && box.height > 0) {
            result.push_back(box);
        }
    }

    return result;
}
