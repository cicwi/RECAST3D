#pragma once

#include <memory>
#include <string>

#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include "node_animation.hpp"
#include "ticker.hpp"
#include "material.hpp"

struct aiMesh;

namespace tomovis {

class Model;
class ShaderProgram;

class Mesh : public Ticker {
  public:
    Mesh(aiMesh* asset_mesh);
    ~Mesh();

    void draw(glm::mat4 world, glm::mat4 model,
                glm::vec3 camera_position, ShaderProgram* program) const;

    void animate(std::vector<PositionKeyframe> positions,
                 std::vector<RotationKeyframe> rotations, float speed,
                 float duration);

    void transform(glm::mat4 transformation);

    void tick(float time_elapsed) override;

    Material& material() { return material_; }
    Material mesh_material() { return mesh_material_; }

    void reset_internal_time() { internal_time_ = 0.0f; }
    void set_visible(bool visible) { visible_ = visible; }

  private:
    friend Model;

    const aiMesh* asset_mesh_;
    Material material_;
    Material mesh_material_;

    GLuint index_handle_;
    unsigned int index_count_ = 0;

    std::vector<PositionKeyframe> positions_;
    std::vector<RotationKeyframe> rotations_;
    float speed_ = 1.0f;
    bool animated_ = false;
    bool visible_ = true;

    float internal_time_ = 0.0f;
    float animation_duration_ = 1.0f;

    glm::mat4 static_mesh_transformation_;
    glm::mat4 mesh_rotate_;
    glm::mat4 mesh_translate_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    GLuint vbo_normals_handle_;
};

} // namespace tomovis
