#include "frame_processor.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace {
std::string modeToString(Mode mode) {
    switch (mode) {
        case Mode::Original: return "Original";
        case Mode::Invert: return "Invert";
        case Mode::Blur: return "Gaussian blur";
        case Mode::Canny: return "Canny";
        case Mode::Sobel: return "Sobel";
        case Mode::Binary: return "Binary";
        case Mode::Glitch: return "Glitch";
    }
    return "Unknown";
}

cv::Mat ensureBgr(const cv::Mat& src) {
    if (src.channels() == 3) {
        return src;
    }
    cv::Mat bgr;
    if (src.channels() == 1) {
        cv::cvtColor(src, bgr, cv::COLOR_GRAY2BGR);
    } else {
        cv::cvtColor(src, bgr, cv::COLOR_BGRA2BGR);
    }
    return bgr;
}
}

cv::Mat FrameProcessor::process(const cv::Mat& input, Mode mode, const FrameSettings& settings) const {
    if (input.empty()) {
        return input;
    }

    cv::Mat frame = ensureBgr(input);
    frame = applyZoom(frame, settings.zoom);
    frame = applyBrightness(frame, settings.brightness);

    switch (mode) {
        case Mode::Original:
            break;
        case Mode::Invert:
            cv::bitwise_not(frame, frame);
            break;
        case Mode::Blur:
            cv::GaussianBlur(frame, frame, cv::Size(15, 15), 0.0);
            break;
        case Mode::Canny: {
            cv::Mat gray, edges;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::Canny(gray, edges, 70, 160);
            cv::cvtColor(edges, frame, cv::COLOR_GRAY2BGR);
            break;
        }
        case Mode::Sobel: {
            cv::Mat gray, gradX, gradY, absX, absY, combined;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::Sobel(gray, gradX, CV_16S, 1, 0, 3);
            cv::Sobel(gray, gradY, CV_16S, 0, 1, 3);
            cv::convertScaleAbs(gradX, absX);
            cv::convertScaleAbs(gradY, absY);
            cv::addWeighted(absX, 0.5, absY, 0.5, 0.0, combined);
            cv::cvtColor(combined, frame, cv::COLOR_GRAY2BGR);
            break;
        }
        case Mode::Binary: {
            cv::Mat gray, binary;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::threshold(gray, binary, 110, 255, cv::THRESH_BINARY);
            cv::cvtColor(binary, frame, cv::COLOR_GRAY2BGR);
            break;
        }
        case Mode::Glitch:
            frame = applyGlitch(frame);
            break;
    }

    if (!settings.drawingLayer.empty()) {
        frame = blendOverlay(frame, settings.drawingLayer);
    }

    drawCrosshair(frame, settings.crosshair);
    drawHud(frame, settings, mode);
    return frame;
}

cv::Mat FrameProcessor::applyZoom(const cv::Mat& input, double zoom) {
    if (zoom <= 1.0) {
        return input.clone();
    }

    const int width = input.cols;
    const int height = input.rows;
    const int cropWidth = std::max(1, static_cast<int>(width / zoom));
    const int cropHeight = std::max(1, static_cast<int>(height / zoom));
    const int x = (width - cropWidth) / 2;
    const int y = (height - cropHeight) / 2;

    cv::Rect roi(x, y, cropWidth, cropHeight);
    roi &= cv::Rect(0, 0, width, height);
    cv::Mat cropped = input(roi).clone();
    cv::Mat resized;
    cv::resize(cropped, resized, input.size(), 0, 0, cv::INTER_LINEAR);
    return resized;
}

cv::Mat FrameProcessor::applyBrightness(const cv::Mat& input, int brightness) {
    const double alpha = 1.0;
    const int beta = brightness - 50;
    cv::Mat adjusted;
    input.convertTo(adjusted, -1, alpha, beta);
    return adjusted;
}

cv::Mat FrameProcessor::applyGlitch(const cv::Mat& input) {
    std::vector<cv::Mat> channels;
    cv::split(input, channels);

    if (channels.size() != 3) {
        return input.clone();
    }

    const int shift = 12;
    if (input.cols <= shift) {
        return input.clone();
    }
    cv::Mat shiftedR = cv::Mat::zeros(channels[2].size(), channels[2].type());
    cv::Mat shiftedG = cv::Mat::zeros(channels[1].size(), channels[1].type());
    cv::Mat shiftedB = cv::Mat::zeros(channels[0].size(), channels[0].type());

    channels[2](cv::Rect(0, 0, channels[2].cols - shift, channels[2].rows))
        .copyTo(shiftedR(cv::Rect(shift, 0, channels[2].cols - shift, channels[2].rows)));
    channels[1](cv::Rect(shift, 0, channels[1].cols - shift, channels[1].rows))
        .copyTo(shiftedG(cv::Rect(0, 0, channels[1].cols - shift, channels[1].rows)));
    channels[0].copyTo(shiftedB);

    std::vector<cv::Mat> merged{shiftedB, shiftedG, shiftedR};
    cv::Mat result;
    cv::merge(merged, result);
    return result;
}

cv::Mat FrameProcessor::blendOverlay(const cv::Mat& base, const cv::Mat& overlay) {
    cv::Mat result = base.clone();
    cv::addWeighted(base, 1.0, overlay, 0.9, 0.0, result);
    return result;
}

void FrameProcessor::drawCrosshair(cv::Mat& frame, const cv::Point& center) {
    cv::Point clamped(
        std::clamp(center.x, 0, frame.cols - 1),
        std::clamp(center.y, 0, frame.rows - 1)
    );
    cv::drawMarker(frame, clamped, cv::Scalar(0, 255, 255), cv::MARKER_CROSS, 20, 2);
}

void FrameProcessor::drawHud(cv::Mat& frame, const FrameSettings& settings, Mode mode) {
    const std::string modeText = "Mode: " + modeToString(mode);
    const std::string fpsText = "FPS: " + cv::format("%.1f", settings.fps);
    const std::string frameText = "Frame: " + std::to_string(settings.frameCounter);
    const std::string zoomText = "Zoom: " + cv::format("%.2f", settings.zoom);

    cv::rectangle(frame, cv::Rect(10, 10, 320, 120), cv::Scalar(0, 0, 0), cv::FILLED);
    cv::putText(frame, modeText, cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, fpsText, cv::Point(20, 65), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
    cv::putText(frame, frameText, cv::Point(20, 90), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
    cv::putText(frame, zoomText, cv::Point(20, 115), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
}
