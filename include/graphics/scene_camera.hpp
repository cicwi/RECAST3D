#pragma once

#include <vector>
#include <utility>
#include <string>

#include <glm/glm.hpp>
#include <GL/gl3w.h>

#include "configurable.hpp"
#include "input_handler.hpp"

namespace tomovis {

class SceneCamera : public InputHandler {
  public:
    SceneCamera();
    virtual ~SceneCamera(){};

    virtual glm::mat4 matrix() = 0;
    virtual std::vector<parameter<float>>& parameters() = 0;

    GLuint colormap() { return colormap_texture_id_; }
    void set_colormap(std::string name);

  private:
    GLuint colormap_texture_id_;
};

} // namespace tomovis
