#pragma once

#include <vector>

#include "graphics/render_target.hpp"
#include "input_handler.hpp"


struct GLFWwindow;

namespace tomovis {

class Window;

class Interface : public RenderTarget, public InputHandler {
  public:
      Interface(GLFWwindow* window);
      ~Interface();

      void render(glm::mat4) override;

      int z_priority() const override {
          return 10;
      }

      void register_window(Window& window);

      bool handle_mouse_button(int button, bool down) override;
      bool handle_scroll(double offset) override;
      bool handle_key(int key, bool down, int mods) override;
      bool handle_char(unsigned int c) override;
      bool handle_mouse_moved(float x, float y) override;

      int priority() const override { return 1; }

  private:
    std::vector<Window*> windows_;
};

} // namespace tomovis
