#pragma once

#include "ticker.hpp"
#include "graphics/interface/window.hpp"

namespace tomovis {

class Recorder : public Ticker, public Window {
  public:
    Recorder();
    ~Recorder();

    void describe();
    void tick(float time_elapsed) override;

  private:
    bool recording_ = false;
};

} // namespace tomovis
