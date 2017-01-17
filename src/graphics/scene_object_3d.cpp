#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/scene_camera_3d.hpp"
#include "graphics/scene_object_3d.hpp"
#include "graphics/shader_program.hpp"

namespace tomovis {

SceneObject3d::SceneObject3d() : SceneObject() {
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

    program_ = std::make_unique<ShaderProgram>("../src/shaders/simple_3d.vert",
                                               "../src/shaders/simple_3d.frag");

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
}

SceneObject3d::~SceneObject3d() {}

void SceneObject3d::update_image_(int slice_idx) {
    slices_[slice_idx]->update_texture();
}

void SceneObject3d::update_slices_() {
    // for (int i = 0; i < (int)orientations.size(); ++i) {
    //    if (slices_[i]->orientation != orientations[i].orientation) {
    //        phantom_slices_[i].active = true;
    //    }
    //    phantom_slices_[i].hovered = orientations[i].hovered;
    //    phantom_slices_[i].orientation = orientations[i].orientation;
    //}

    // TODO: if they do not match, send a request for new slice information
    // and wait until set_data is called with the new slice data (how to detect)
    // for this we need:
    // - some kind of publishing system, no longer master-slave
}

void SceneObject3d::draw(glm::mat4 window_matrix) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // update slices
    // update_slices_(camera_3d_->get_slices());

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

        GLint has_data_loc = glGetUniformLocation(program_->handle(), "has_data");
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
        if(rhs->hovered == lhs->hovered) {
            return rhs->id_ < lhs->id_;
        }
        return rhs->hovered;
    });

    for (auto& slice : slices) {
        draw_slice(*slice);
    }

    // TODO low-resolution transparent voxel volume if available?

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

}  // namespace tomovis
