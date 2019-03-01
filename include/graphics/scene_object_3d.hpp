#pragma once

#include <memory>

#include "scene_camera_3d.hpp"
#include "scene_object.hpp"
#include "slice.hpp"

namespace tomovis {

class SceneObject3d : public SceneObject {
   public:
    SceneObject3d(int scene_id);
    ~SceneObject3d();

    void draw(glm::mat4 window_matrix) override;
};

}  // namespace tomovis
