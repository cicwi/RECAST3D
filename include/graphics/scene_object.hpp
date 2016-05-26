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

  private:
    GLuint vao_handle_;
    GLuint vbo_handle_[2];
    std::unique_ptr<ShaderProgram> program_;
};

} // namespace tomovis
