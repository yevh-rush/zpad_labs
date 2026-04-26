#pragma once

enum class Mode {
    Original,
    Invert,
    Blur,
    Canny,
    Sobel,
    Binary,
    Glitch
};

class KeyProcessor {
public:
    KeyProcessor();

    bool processKey(int key);
    Mode mode() const;
    bool paused() const;
    bool quitRequested() const;

private:
    Mode mode_;
    bool paused_;
    bool quitRequested_;
};
