#include "key_processor.hpp"

KeyProcessor::KeyProcessor()
    : mode_(Mode::Original), paused_(false), quitRequested_(false) {}

bool KeyProcessor::processKey(int key) {
    if (key < 0) {
        return quitRequested_;
    }

    switch (key) {
        case '1': mode_ = Mode::Original; break;
        case '2': mode_ = Mode::Invert; break;
        case '3': mode_ = Mode::Blur; break;
        case '4': mode_ = Mode::Canny; break;
        case '5': mode_ = Mode::Sobel; break;
        case '6': mode_ = Mode::Binary; break;
        case '7': mode_ = Mode::Glitch; break;
        case 'f':
        case 'F': mode_ = Mode::Face; break;
        case ' ':
            paused_ = !paused_;
            break;
        case 'q':
        case 'Q':
        case 27:
            quitRequested_ = true;
            break;
        default:
            break;
    }

    return quitRequested_;
}

Mode KeyProcessor::mode() const {
    return mode_;
}

bool KeyProcessor::paused() const {
    return paused_;
}

bool KeyProcessor::quitRequested() const {
    return quitRequested_;
}
