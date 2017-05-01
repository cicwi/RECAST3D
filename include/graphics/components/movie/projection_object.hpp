#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>

#include "graphics/shader_program.hpp"
#include "ticker.hpp"

namespace tomovis {

class Model;

class ProjectionObject : public Ticker {
  public:
    ProjectionObject();
    ~ProjectionObject();

    void draw(glm::mat4 world_to_screen, const Model& model) const;
    void tick(float time_elapsed) override;

  private:
    bool initialize_fbo_();
    void bind_fbo_texture_() const;
    void draw_tomo_(glm::mat4 world_to_detector, const Model& model) const;

    glm::vec3 source_;
    glm::vec3 detector_base_;
    glm::vec3 detector_axis1_;
    glm::vec3 detector_axis2_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;

    GLuint fbo_handle_;
    GLuint fbo_texture_;

    glm::mat4 orientation_matrix_;
};

} // namespace tomovis
