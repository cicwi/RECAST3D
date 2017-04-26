#pragma once

#include <memory>
#include <string>

#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include "node_animation.hpp"
#include "shader_program.hpp"
#include "ticker.hpp"

struct aiMesh;

namespace tomovis {

class Mesh : public Ticker {
  public:
    Mesh(aiMesh* asset_mesh);
    ~Mesh();

    void draw(glm::mat4 world_to_screen, glm::mat4 model,
              glm::vec3 camera_position) const;

    void animate(std::vector<PositionKeyframe> positions,
                 std::vector<RotationKeyframe> rotations, float speed,
                 float duration);

    void tick(float time_elapsed) override;

  private:
    const aiMesh* asset_mesh_;

    GLuint index_handle_;
    unsigned int index_count_;

    std::vector<PositionKeyframe> positions_;
    std::vector<RotationKeyframe> rotations_;
    float speed_ = 1.0f;
    bool animated_ = false;

    float internal_time_ = 0.0f;
    float animation_duration_ = 1.0f;

    glm::mat4 mesh_matrix_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    GLuint vbo_normals_handle_;
    // FIXME: replace with single program
    std::unique_ptr<ShaderProgram> program_;
};

} // namespace tomovis
