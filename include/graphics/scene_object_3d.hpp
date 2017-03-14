#pragma once

#include <memory>

#include "scene_camera_3d.hpp"
#include "scene_object.hpp"
#include "slice.hpp"

namespace tomovis {

class SceneObject3d : public SceneObject {
   public:
    SceneObject3d();
    ~SceneObject3d();

    void draw(glm::mat4 window_matrix) override;

    SceneCamera& camera() override { return *camera_3d_; }

   private:
    // overwrite specific (3d) camera to use
    std::unique_ptr<SceneCamera3d> camera_3d_;
    std::map<int, std::unique_ptr<slice>> slices_;
};

}  // namespace tomovis
