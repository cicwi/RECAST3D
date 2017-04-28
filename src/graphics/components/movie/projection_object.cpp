#include "graphics/components/movie/projection_object.hpp"

namespace tomovis {

ProjectionObject::ProjectionObject() {
    static const GLfloat square[4][3] = {{0.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 0.0f},
                                         {1.0f, 0.0f, 0.0f}};

    glGenVertexArrays(1, &vao_handle_);
    glBindVertexArray(vao_handle_);
    glGenBuffers(1, &vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    program_ = std::make_unique<ShaderProgram>("../src/shaders/screen.vert",
                                               "../src/shaders/screen.frag");
}

ProjectionObject::~ProjectionObject() {}

void ProjectionObject::draw(glm::mat4 world_to_screen) const {
    glEnable(GL_DEPTH_TEST);

    program_->use();

    //        program_->uniform("texture_sampler", 0);
    //        proj.data_texture.bind();

    glm::mat4 orientation_matrix;
    program_->uniform("orientation_matrix", orientation_matrix);
    program_->uniform("world_to_screen_matrix", world_to_screen);

    glBindVertexArray(vao_handle_);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisable(GL_DEPTH_TEST);
}

void ProjectionObject::tick(float time_elapsed) { (void)time_elapsed; }

} // namespace tomovis
