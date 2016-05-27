#pragma once

#include <memory>

#include <GL/gl3w.h>
#include <glm/glm.hpp>


namespace tomovis {

class ShaderProgram;
class SceneCamera;

class SceneObject {
  public:
    SceneObject();
    virtual ~SceneObject();

    virtual void draw(glm::mat4 window_matrix) = 0;

    float& size() { return size_; }
    SceneCamera& camera() { return *camera_; }

  protected:
    GLuint vao_handle_;
    GLuint vbo_handle_[2];
    std::unique_ptr<ShaderProgram> program_;
    std::unique_ptr<SceneCamera> camera_;
    float size_ = 1.0;
};

} // namespace tomovis
