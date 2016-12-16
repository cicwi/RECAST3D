#include <memory>

#include <GL/gl3w.h>

#include "graphics/scene_object.hpp"
#include "graphics/shader_program.hpp"

#include "graphics/scene_camera.hpp"

namespace tomovis {

SceneObject::SceneObject() : size_{32, 32} {
    static const GLfloat square[4][2] = {
        {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0}};

    glGenVertexArrays(1, &vao_handle_);
    glBindVertexArray(vao_handle_);

    glGenBuffers(1, &vbo_handle_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    program_ = std::make_unique<ShaderProgram>("src/shaders/simple.vert",
                                               "src/shaders/simple.frag");
}

SceneObject::~SceneObject() {
    glDeleteVertexArrays(1, &vao_handle_);
    glDeleteBuffers(1, &vbo_handle_);
}

}  // namespace tomovis
