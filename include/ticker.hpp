#pragma once

namespace tomovis {

class Ticker {
  public:
      virtual void tick(float time_elapsed) = 0;
};

} // namespace tomovis
