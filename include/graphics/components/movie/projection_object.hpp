#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>

#include "graphics/shader_program.hpp"
#include "ticker.hpp"

namespace tomovis {

class ProjectionObject : public Ticker {
  public:
    ProjectionObject();
    ~ProjectionObject();

    void draw(glm::mat4 world_to_screen) const;
    void tick(float time_elapsed) override;

  private:
    glm::vec3 source_;
    glm::vec3 detector_base_;
    glm::vec3 detector_axis1_;
    glm::vec3 detector_axis2_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;

    glm::mat4 orientation_matrix_;
};

} // namespace tomovis
