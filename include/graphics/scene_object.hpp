#pragma once

#include <memory>
#include <vector>

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

    float& pixel_size() { return pixel_size_; }
    virtual SceneCamera& camera() { return *camera_; }

    virtual void set_data(std::vector<unsigned char>& data, int slice = 0) = 0;
    virtual void set_size(std::vector<int>& size, int slice = 0) = 0;

  protected:
    virtual void update_image_(int slice = 0) = 0;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;
    std::unique_ptr<SceneCamera> camera_;
    float pixel_size_ = 1.0;


};

} // namespace tomovis
