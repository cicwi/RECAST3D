#include "graphics/scene_object_2d.hpp"
#include "graphics/scene_camera_2d.hpp"
#include "graphics/shader_program.hpp"


namespace tomovis {

SceneObject2d::SceneObject2d() : SceneObject() {
    unsigned char image[count_ * count_ * 3];
    for (int i = 0; i < count_ * count_ * 3; ++i) {
        image[i] = (i % 4 == 0) ?  255 : 0;
    }

    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, count_, count_, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    camera_ = std::make_unique<SceneCamera2d>();
}

SceneObject2d::~SceneObject2d() {}

void SceneObject2d::draw(glm::mat4 window_matrix) {
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    program_->use();
    GLint loc = glGetUniformLocation(program_->handle(), "size");
    glUniform1f(loc, (1.0 / 20.0) * size_);

    GLint matrix_loc =
        glGetUniformLocation(program_->handle(), "transform_matrix");
    auto transform_matrix = window_matrix * camera_->matrix();
    glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, &transform_matrix[0][0]);

    glBindVertexArray(vao_handle_);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 400);  
}

} // namespace tomovis
