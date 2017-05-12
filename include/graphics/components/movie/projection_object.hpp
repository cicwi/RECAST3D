#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>

#include "graphics/interface/window.hpp"
#include "graphics/shader_program.hpp"
#include "ticker.hpp"

namespace tomovis {

class Model;

class ProjectionObject : public Ticker, public Window {
  public:
    ProjectionObject();
    ~ProjectionObject();

    void describe();
    void draw(glm::mat4 world_to_screen, const Model& model,
              glm::vec3 camera_position) const;
    void tick(float time_elapsed) override;

    glm::vec3& source() { return source_; }

  private:
    bool initialize_fbo_();
    void bind_fbo_texture_() const;
    void draw_tomo_(const Model& model) const;
    glm::mat4 beam_transform_() const;
    glm::vec3 detector_center_() const;

    glm::vec3 source_;
    glm::vec3 detector_base_;
    glm::vec3 detector_axis1_;
    glm::vec3 detector_axis2_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;

    GLuint fbo_handle_;
    GLuint fbo_texture_;

    std::unique_ptr<ShaderProgram> shadow_program_;

    glm::mat4 orientation_matrix_;

    std::unique_ptr<ShaderProgram> beam_program_;
    GLuint cube_vao_handle_;
    GLuint cube_vbo_handle_;
};

} // namespace tomovis
