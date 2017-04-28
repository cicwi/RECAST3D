#pragma once

#include <array>
#include <memory>
#include <glm/glm.hpp>

#include "ticker.hpp"
#include "graphics/shader_program.hpp"

namespace tomovis {

class ProjectionObject : public Ticker {
  public:
    ProjectionObject();
    ~ProjectionObject();

    void draw(glm::mat4 world_to_screen) const;
    void tick(float time_elapsed) override;

  private:
    glm::vec3 source_;
    glm::vec3 detector_min_;
    glm::vec3 detector_max_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;
};

} // namespace tomovis
