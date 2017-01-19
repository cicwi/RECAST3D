#pragma once

#include <map>

#include "scene_camera_3d.hpp"
#include "scene_object.hpp"
#include "slice.hpp"

namespace tomovis {

class SceneObject3d : public SceneObject {
   public:
    SceneObject3d();
    ~SceneObject3d();

    void draw(glm::mat4 window_matrix) override;

    void set_data(std::vector<unsigned char>& data, int slice = 0) override {
        if (slice < 0 || slice >= (int)slices_.size()) throw;

        slices_[slice]->data = data;
        update_image_(slice);
    }

    SceneCamera& camera() override { return *camera_3d_; }

    void set_size(std::vector<int>& size, int slice = 0) override {
        if (slice < 0 || slice >= (int)slices_.size()) throw;
        slices_[slice]->size = size;
    }

   protected:
    void update_image_(int slice = 0) override;

   private:
    // overwrite specific (3d) camera to use
    std::unique_ptr<SceneCamera3d> camera_3d_;
    std::map<int, std::unique_ptr<slice>> slices_;

    glm::vec3 box_origin_;
    glm::vec3 box_size_;

    GLuint cube_vao_handle_;
    GLuint cube_vbo_handle_;
    std::unique_ptr<ShaderProgram> cube_program_;
};

}  // namespace tomovis
