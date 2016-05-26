#pragma once

#include <vector>

#include "graphics/render_target.hpp"


struct GLFWwindow;

namespace tomovis {

class Window;

class Interface : public RenderTarget {
  public:
      Interface(GLFWwindow* window);
      ~Interface();

      void render() override;

      int z_priority() const override {
          return 10;
      }

      void register_window(Window& window);

  private:
    std::vector<Window*> windows_;
};

} // namespace tomovis
