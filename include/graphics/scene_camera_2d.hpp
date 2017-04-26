#pragma once

#include <glm/glm.hpp>
#include "graphics/scene_camera.hpp"

namespace tomovis {

class SceneCamera2d : public SceneCamera {
   public:
    SceneCamera2d();

    glm::mat4 matrix() override;

    std::vector<parameter<float>>& parameters() override { return parameters_; }

    bool handle_mouse_button(int button, bool down) override;
    bool handle_scroll(double offset) override;
    bool handle_mouse_moved(float x, float y) override;
    bool handle_key(int key, bool down, int mods) override;

    glm::vec3 position() override { return glm::vec3(0.0f); }
    glm::vec3 look_at() override { return glm::vec3(0.0f); }

   private:
    glm::vec2 position_;
    std::vector<parameter<float>> parameters_;

    float angle_ = 0.0f;
    float scale_ = 0.5f;

    float prev_x_ = -1.1f;
    float prev_y_ = -1.1f;

    bool dragging_ = false;
};

}  // namespace tomovis
