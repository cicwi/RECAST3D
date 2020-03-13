#include <iostream>

#include "graphics/scene_camera_2d.hpp"
#include "graphics/scene_object_2d.hpp"
#include "graphics/shader_program.hpp"

namespace tomovis {

SceneObject2d::SceneObject2d(int scene_id) : SceneObject(scene_id), size_{32, 32} {
    static const GLfloat square[4][2] = {{-1.0f, -1.0f},
                                         {-1.0f, 1.0f},
                                         {1.0f, 1.0f},
                                         {1.0f, -1.0f}};

    glGenVertexArrays(1, &vao_handle_);
    glBindVertexArray(vao_handle_);

    glGenBuffers(1, &vbo_handle_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    program_ = std::make_unique<ShaderProgram>("../src/shaders/simple.vert",
                                               "../src/shaders/simple.frag");
    glGenTextures(1, &texture_id_);

    data_.resize(size_[0] * size_[1]);
    for (int i = 0; i < size_[0] * size_[1]; ++i) {
        data_[i] = i % 256;
    }
    update_image_();

    camera_ = std::make_unique<SceneCamera2d>();
}

SceneObject2d::~SceneObject2d() {}

void SceneObject2d::update_image_(int slice) {
    if (slice != 0)
        throw;

    glBindTexture(GL_TEXTURE_2D, texture_id_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size_[0], size_[1], 0, GL_RED,
                 GL_UNSIGNED_BYTE, data_.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void SceneObject2d::draw(glm::mat4 window_matrix) {
    program_->use();

    glUniform1i(glGetUniformLocation(program_->handle(), "texture_sampler"), 0);
    glUniform1i(glGetUniformLocation(program_->handle(), "colormap_sampler"),
                1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, camera_->colormap());

    GLint loc = glGetUniformLocation(program_->handle(), "size");
    glUniform1f(loc, (1.0 / 20.0) * pixel_size_);

    auto asp = (float)size_[0] / size_[1];
    GLint loc_asp = glGetUniformLocation(program_->handle(), "aspect_ratio");
    glUniform1f(loc_asp, asp);
    GLint loc_iasp =
        glGetUniformLocation(program_->handle(), "inv_aspect_ratio");
    glUniform1f(loc_iasp, 1.0f / asp);

    GLint matrix_loc =
        glGetUniformLocation(program_->handle(), "transform_matrix");
    auto transform_matrix = window_matrix * camera_->matrix();
    glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, &transform_matrix[0][0]);

    glBindVertexArray(vao_handle_);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

}  // namespace tomovis
