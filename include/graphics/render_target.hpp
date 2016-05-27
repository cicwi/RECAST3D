#pragma once

#include <glm/glm.hpp>


namespace tomovis {

class RenderTarget {
  public:
    virtual void render(glm::mat4 window_matrix) = 0;
    virtual int z_priority() const { return 0; }
};

} // namespace tomovis
