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

  private:
    glm::vec2 position_;
    std::vector<parameter<float>> parameters_;

    float angle_;
    float scale_;
};

} // namespace tomovis
