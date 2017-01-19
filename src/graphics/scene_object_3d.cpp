#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/scene_camera_3d.hpp"
#include "graphics/scene_object_3d.hpp"
#include "graphics/shader_program.hpp"

namespace tomovis {

SceneObject3d::SceneObject3d() : SceneObject() {
    //FIXME move all this primitives stuff to a separate file
    static const GLfloat square[4][3] = {{0.0f, 0.0f, 1.0f},
                                         {0.0f, 1.0f, 1.0f},
                                         {1.0f, 1.0f, 1.0f},
                                         {1.0f, 0.0f, 1.0f}};

    glGenVertexArrays(1, &vao_handle_);
    glBindVertexArray(vao_handle_);
    glGenBuffers(1, &vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    static const GLfloat cube[] = {
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,
        1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,
        1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
        1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f};

    glGenVertexArrays(1, &cube_vao_handle_);
    glBindVertexArray(cube_vao_handle_);
    glGenBuffers(1, &cube_vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 9 * 12 * sizeof(GLfloat), cube,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    program_ = std::make_unique<ShaderProgram>("../src/shaders/simple_3d.vert",
                                               "../src/shaders/simple_3d.frag");

    cube_program_ =
        std::make_unique<ShaderProgram>("../src/shaders/wireframe_cube.vert",
                                        "../src/shaders/wireframe_cube.frag");

    slices_[0] = std::make_unique<slice>(0);
    slices_[1] = std::make_unique<slice>(1);
    slices_[2] = std::make_unique<slice>(2);

    // slice along axis 0 = x
    slices_[0]->set_orientation(glm::vec3(0.0f, -1.0f, -1.0f),
                                glm::vec3(0.0f, 2.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 2.0f));

    // slice along axis 1 = y
    slices_[1]->set_orientation(glm::vec3(-1.0f, 0.0f, -1.0f),
                                glm::vec3(2.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 2.0f));

    // slice along axis 2 = z
    slices_[2]->set_orientation(glm::vec3(-1.0f, -1.0f, 0.0f),
                                glm::vec3(2.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 2.0f, 0.0f));

    std::vector<glm::mat4> orientations;
    for (auto& slice : slices_) {
        orientations.push_back(slice.second->orientation);
    }
    camera_3d_ = std::make_unique<SceneCamera3d>(slices_);

    box_origin_ = glm::vec3(-1.0f);
    box_size_ = glm::vec3(2.0f);
}

SceneObject3d::~SceneObject3d() {
    glDeleteVertexArrays(1, &cube_vao_handle_);
    glDeleteBuffers(1, &cube_vbo_handle_);
}

void SceneObject3d::update_image_(int slice_idx) {
    slices_[slice_idx]->update_texture();
}

void SceneObject3d::draw(glm::mat4 window_matrix) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    program_->use();

    glUniform1i(glGetUniformLocation(program_->handle(), "texture_sampler"), 0);
    glUniform1i(glGetUniformLocation(program_->handle(), "colormap_sampler"),
                1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, camera_3d_->colormap());

    auto draw_slice = [&](slice& slice) {
        slice.get_texture().bind();

        GLint matrix_loc =
            glGetUniformLocation(program_->handle(), "transform_matrix");
        auto transform_matrix =
            window_matrix * camera_3d_->matrix() * slice.orientation;
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, &transform_matrix[0][0]);

        GLint hovered_loc = glGetUniformLocation(program_->handle(), "hovered");
        glUniform1i(hovered_loc, (int)(slice.hovered));

        GLint has_data_loc =
            glGetUniformLocation(program_->handle(), "has_data");
        glUniform1i(has_data_loc, (int)(slice.has_data()));

        glBindVertexArray(vao_handle_);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        slice.get_texture().unbind();
    };

    std::vector<slice*> slices;
    for (auto& id_slice : slices_) {
        slices.push_back(id_slice.second.get());
    }
    std::sort(slices.begin(), slices.end(), [](auto& lhs, auto& rhs) -> bool {
        if (rhs->transparent() == lhs->transparent()) {
            return rhs->id_ < lhs->id_;
        }
        return rhs->transparent();
    });

    for (auto& slice : slices) {
        draw_slice(*slice);
    }

    cube_program_->use();

    auto transform_matrix = window_matrix * camera_3d_->matrix();
    GLint matrix_loc =
        glGetUniformLocation(cube_program_->handle(), "transform_matrix");
    glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, &transform_matrix[0][0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(cube_vao_handle_);
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // TODO low-resolution transparent voxel volume if available?

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

}  // namespace tomovis
