#pragma once

#include <memory>
#include <string>

#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include "graphics/shader_program.hpp"
#include "render_target.hpp"

struct aiMesh;

namespace tomovis {

class Mesh {
  public:
    Mesh(aiMesh* asset_mesh);
    ~Mesh();

    void draw(glm::mat4 model_to_screen) const;

  private:
    const aiMesh* asset_mesh_;

    GLuint index_handle_;
    unsigned int index_count_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    GLuint vbo_normals_handle_;
    // FIXME: replace with single program
    std::unique_ptr<ShaderProgram> program_;
};

} // namespace tomovis
