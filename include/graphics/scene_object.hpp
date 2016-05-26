#pragma once

#include <memory>

#include <GL/gl3w.h>


namespace tomovis {

class ShaderProgram;

class SceneObject {
  public:
    SceneObject();
    virtual ~SceneObject();

    virtual void draw();

    float& size() { return size_; }

  protected:
    GLuint vao_handle_;
    GLuint vbo_handle_[2];
    std::unique_ptr<ShaderProgram> program_;
    float size_ = 1.0;
};

} // namespace tomovis
