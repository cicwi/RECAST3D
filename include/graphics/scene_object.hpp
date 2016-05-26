#pragma once

#include <GL/gl3w.h>

namespace tomovis {

class SceneObject {
  public:
    SceneObject();
    virtual ~SceneObject();

  private:
    GLuint vao_handle_;
};

} // namespace tomovis
