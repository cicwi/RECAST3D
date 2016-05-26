#pragma once

#include "graphics/render_target.hpp"


struct GLFWwindow;

namespace tomovis {

class Interface : public RenderTarget {
  public:
      Interface(GLFWwindow* window);
      ~Interface();

      void render() override;

      int z_priority() const override {
          return 10;
      }
};

} // namespace tomovis
