#pragma once

#include <opencv2/opencv.hpp>
#include "key_processor.hpp"

struct FrameSettings {
    cv::Point crosshair{320, 240};
    double zoom{1.0};
    int brightness{50};
    int frameCounter{0};
    double fps{0.0};
    cv::Mat drawingLayer;
    cv::Point mouseAnchor{-1, -1};
    cv::Point mouseCurrent{-1, -1};
    bool draggingLine{false};
    bool draggingRect{false};
};

class FrameProcessor {
public:
    cv::Mat process(const cv::Mat& input, Mode mode, const FrameSettings& settings) const;

private:
    static cv::Mat applyZoom(const cv::Mat& input, double zoom);
    static cv::Mat applyBrightness(const cv::Mat& input, int brightness);
    static cv::Mat applyGlitch(const cv::Mat& input);
    static cv::Mat blendOverlay(const cv::Mat& base, const cv::Mat& overlay);
    static void drawHud(cv::Mat& frame, const FrameSettings& settings, Mode mode);
    static void drawCrosshair(cv::Mat& frame, const cv::Point& center);
};
