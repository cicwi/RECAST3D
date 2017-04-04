#include <iostream>

#include <glm/gtx/transform.hpp>

#include "graphics/components/partitioning_component.hpp"

namespace tomovis {

inline glm::vec3 hue_to_rgb(float hue) {
    int h = (int)(hue * 360);
    auto c = 1.0f;
    auto x = c * (1.0f - glm::abs((h / 60 % 2) - 1));

    if (h < 60) {
        return {c, x, 0};
    } else if (h < 120) {
        return {x, c, 0};
    } else if (h < 180) {
        return {0, c, x};
    } else if (h < 240) {
        return {0, x, c};
    } else if (h < 300) {
        return {x, 0, c};
    } else {
        return {c, 0, x};
    }
}

PartitioningComponent::PartitioningComponent(SceneObject& object, int scene_id)
    : object_(object), scene_id_(scene_id) {
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

    part_program_ = std::make_unique<ShaderProgram>("../src/shaders/part.vert",
                                                    "../src/shaders/part.frag");

    parts_.push_back(part(0, {-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}));
    parts_.push_back(part(1, {0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}));
}

PartitioningComponent::~PartitioningComponent() {}

void PartitioningComponent::draw(glm::mat4 world_to_screen) const {
    if (parts_.empty()) {
        return;
    }

    glEnable(GL_DEPTH_TEST);

    auto draw_part = [&](auto& the_part) {
        part_program_->use();

        glm::mat4 object_matrix =
            glm::translate(-0.5f * (the_part.max_pt + the_part.min_pt)) *
            glm::scale(global_scale_ * 0.5f * (the_part.max_pt - the_part.min_pt));

        auto transform_matrix = world_to_screen * object_matrix;

        GLint matrix_loc =
            glGetUniformLocation(part_program_->handle(), "transform_matrix");
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, &transform_matrix[0][0]);

        auto color = hue_to_rgb((float)the_part.id / parts_.size());
        GLint rgb_loc =
            glGetUniformLocation(part_program_->handle(), "rgb_color");
        glUniform3fv(rgb_loc, 1, &color[0]);

        glBindVertexArray(cube_vao_handle_);
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
    };

    for (auto& the_part : parts_) {
        draw_part(the_part);
    }

    glDisable(GL_DEPTH_TEST);
}

}  // namespace tomovis
