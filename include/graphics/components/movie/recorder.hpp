#pragma once

#include <stdio.h>

#include "ticker.hpp"
#include "graphics/interface/window.hpp"

namespace tomovis {

class Recorder : public Window {
  public:
    Recorder();
    ~Recorder();

    void describe();
    void capture();

    void start();
    void stop();

  private:
    bool recording_ = false;
    FILE* ffmpeg_ = nullptr;
    int* buffer_ = nullptr;

    int width_ = 1024;
    int height_ = 768;
};

} // namespace tomovis
