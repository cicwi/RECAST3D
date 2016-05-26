#pragma once


namespace tomovis {

class RenderTarget {
  public:
    virtual void render() = 0;
    virtual int z_priority() const { return 0; }
};

} // namespace tomovis
