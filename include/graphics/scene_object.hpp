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
    SceneCamera& camera() { return *camera_; }

    void set_data(std::vector<unsigned char>& data) {
        data_ = data;
        update_image_();
    }

    void set_size(std::vector<int>& size) {
        size_ = size;
    }

  protected:
    virtual void update_image_() = 0;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;
    std::unique_ptr<SceneCamera> camera_;
    float pixel_size_ = 1.0;

    std::vector<unsigned char> data_;
    std::vector<int> size_;

};

} // namespace tomovis
