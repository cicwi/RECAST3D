#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "graphics/shader_program.hpp"

#include "object_component.hpp"

namespace tomovis {

class SceneObject;

class AxesComponent : public ObjectComponent {
   public:
    AxesComponent(SceneObject& object, int scene_id);

    void draw(glm::mat4 world_to_screen) override;
    std::string identifier() const override { return "axes"; }

    void describe() override;

   private:
    SceneObject& object_;
    int scene_id_;
    
    GLuint axes_vao_handle_;
    GLuint axes_vbo_handle_;
    GLuint axes_index_handle_;
    int axes_index_count_;
    std::unique_ptr<ShaderProgram> axes_program_;

    bool show_ = true;
};

}  // namespace tomovis
