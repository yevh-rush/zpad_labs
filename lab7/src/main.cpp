#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>

#include "camera_provider.hpp"
#include "display.hpp"
#include "face_detector.hpp"
#include "frame_processor.hpp"
#include "key_processor.hpp"

namespace {
struct MouseState {
    FrameSettings* settings;
};

void onMouse(int event, int x, int y, int flags, void* userdata) {
    auto* state = static_cast<MouseState*>(userdata);
    auto* settings = state->settings;

    settings->mouseCurrent = {x, y};

    if (event == cv::EVENT_LBUTTONDOWN) {
        settings->draggingLine = true;
        settings->mouseAnchor = {x, y};
    } else if (event == cv::EVENT_MOUSEMOVE && settings->draggingLine) {
        settings->mouseCurrent = {x, y};
        if (!settings->drawingLayer.empty()) {
            cv::line(settings->drawingLayer, settings->mouseAnchor, settings->mouseCurrent, cv::Scalar(255, 0, 0), 2);
            settings->mouseAnchor = settings->mouseCurrent;
        }
    } else if (event == cv::EVENT_LBUTTONUP && settings->draggingLine) {
        if (!settings->drawingLayer.empty()) {
            cv::line(settings->drawingLayer, settings->mouseAnchor, {x, y}, cv::Scalar(255, 0, 0), 2);
        }
        settings->draggingLine = false;
        settings->mouseAnchor = {-1, -1};
    } else if (event == cv::EVENT_RBUTTONDOWN) {
        settings->draggingRect = true;
        settings->mouseAnchor = {x, y};
    } else if (event == cv::EVENT_RBUTTONUP && settings->draggingRect) {
        if (!settings->drawingLayer.empty()) {
            cv::rectangle(settings->drawingLayer, settings->mouseAnchor, {x, y}, cv::Scalar(0, 255, 0), 2);
        }
        settings->draggingRect = false;
        settings->mouseAnchor = {-1, -1};
    } else if (event == cv::EVENT_MOUSEWHEEL) {
        const int delta = cv::getMouseWheelDelta(flags);
        if (delta > 0) {
            settings->zoom = std::min(3.0, settings->zoom + 0.1);
        } else if (delta < 0) {
            settings->zoom = std::max(1.0, settings->zoom - 0.1);
        }
    }
}

void moveCrosshair(FrameSettings& settings, int dx, int dy) {
    settings.crosshair.x = std::max(0, settings.crosshair.x + dx);
    settings.crosshair.y = std::max(0, settings.crosshair.y + dy);
}
}

int main(int argc, char** argv) {
    const int cameraIndex = (argc > 1) ? std::stoi(argv[1]) : 0;
    CameraProvider camera(cameraIndex);
    if (!camera.isOpened()) {
        std::cerr << "Cannot open camera index " << cameraIndex << std::endl;
        return 1;
    }

    FaceDetector faceDetector("models/deploy.prototxt", "models/res10_300x300_ssd_iter_140000.caffemodel");
    if (!faceDetector.isReady()) {
        std::cerr << "Face detector model is not ready." << std::endl;
    }

    Display display("Lab 07 - Face detection");
    KeyProcessor keyProcessor;
    FrameProcessor frameProcessor;
    FrameSettings settings;

    cv::Mat firstFrame = camera.getFrame();
    if (firstFrame.empty()) {
        std::cerr << "Failed to read first frame." << std::endl;
        return 1;
    }

    settings.drawingLayer = cv::Mat::zeros(firstFrame.size(), CV_8UC3);
    settings.crosshair = {firstFrame.cols / 2, firstFrame.rows / 2};

    int brightnessTrackbar = 50;
    cv::namedWindow(display.windowName(), cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Brightness", display.windowName(), &brightnessTrackbar, 100);

    MouseState mouseState{&settings};
    cv::setMouseCallback(display.windowName(), onMouse, &mouseState);

    auto lastTick = std::chrono::steady_clock::now();

    while (true) {
        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - lastTick).count();
        lastTick = now;
        if (elapsed > 0.0) {
            settings.fps = 1.0 / elapsed;
        }

        settings.brightness = brightnessTrackbar;

        if (!keyProcessor.paused()) {
            cv::Mat frame = camera.getFrame();
            if (frame.empty()) {
                std::cerr << "Empty frame received" << std::endl;
                continue;
            }

            if (settings.drawingLayer.empty() || settings.drawingLayer.size() != frame.size()) {
                settings.drawingLayer = cv::Mat::zeros(frame.size(), CV_8UC3);
                settings.crosshair = {frame.cols / 2, frame.rows / 2};
            }

            if (keyProcessor.mode() == Mode::Face) {
                settings.faceBoxes = faceDetector.detect(frame);
            } else {
                settings.faceBoxes.clear();
            }

            ++settings.frameCounter;
            cv::Mat processed = frameProcessor.process(frame, keyProcessor.mode(), settings);
            display.show(processed);
        }

        int key = cv::waitKey(1);
        if (key == 81 || key == 'a' || key == 'A') moveCrosshair(settings, -10, 0);
        if (key == 82 || key == 'w' || key == 'W') moveCrosshair(settings, 0, -10);
        if (key == 83 || key == 'd' || key == 'D') moveCrosshair(settings, 10, 0);
        if (key == 84 || key == 's' || key == 'S') moveCrosshair(settings, 0, 10);
        if (key == 'r' || key == 'R' || key == 'c' || key == 'C') {
            settings.drawingLayer = cv::Mat::zeros(settings.drawingLayer.size(), CV_8UC3);
        }

        if (keyProcessor.processKey(key)) {
            break;
        }
    }

    return 0;
}
