#pragma once

#include <glm/glm.hpp>
#include "graphics/scene_camera.hpp"


namespace tomovis {

class SceneCamera2d : public SceneCamera {
  public:
    SceneCamera2d();

    glm::mat4 matrix() override;

    std::vector<parameter<float>>& parameters() override {
        return parameters_;
    }


    bool handle_mouse_button(int button, bool down) override;
    bool handle_scroll(double offset) override;
    bool handle_key(int key, bool down, int mods) override;
    bool handle_char(unsigned int c) override;

  private:
    glm::vec2 position_;
    std::vector<parameter<float>> parameters_;

    float angle_;
    float scale_;

    bool dragging_ = false;
};

} // namespace tomovis
