#include <iostream>
#include <limits>

#include "graphics/scene_camera_3d.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

namespace tomovis {

SceneCamera3d::SceneCamera3d() {
    parameters_.push_back({"angle", 0.0f, 2.0f * M_PI, &angle_});
    parameters_.push_back({"x", -10.0f, 10.0f, &position_.x});
    parameters_.push_back({"y", -10.0f, 10.0f, &position_.y});
    parameters_.push_back({"z", -10.0f, 10.0f, &position_.z});
    parameters_.push_back({"scale", 0.0f, 1.0f, &scale_});

    position_.z = -5.0f;

    up_.y = 1.0f;
    right_.x = 1.0f;

    reset_view();
    switch_if_necessary(drag_machine_kind::rotator);
}

void SceneCamera3d::reset_view() {
    // explicitely set to identity
    view_matrix_ = glm::mat4();
    view_matrix_ =
        glm::rotate(-0.5f * glm::pi<float>(), right_) * view_matrix_;
    view_matrix_ =
        glm::rotate(-0.25f * glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0)) * view_matrix_;
    view_matrix_ =
        glm::rotate(-0.125f * glm::pi<float>(), glm::vec3(1.0, 0.0, 0.0)) * view_matrix_;
}

void SceneCamera3d::look_at(glm::vec3 center) {
    center_ = center;
}

glm::mat4 SceneCamera3d::matrix() {
    glm::mat4 camera_matrix = view_matrix_;

    camera_matrix =
        glm::lookAt(position_, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
         camera_matrix * glm::translate(-center_);

    camera_matrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f) *
                    camera_matrix;

    return camera_matrix;
}

bool SceneCamera3d::handle_mouse_button(int /* button */, bool down) {
    dragging_ = down;
    return true;
}

bool SceneCamera3d::handle_scroll(double offset) {
    position_ *= (1.0 - offset / 20.0);
    return true;
}

bool SceneCamera3d::handle_key(int key, bool down, int /* mods */) {
    float offset = 0.05f;
    if (down) {
        switch (key) {
            case GLFW_KEY_H:
                position_.x -= offset;
                return true;
            case GLFW_KEY_L:
                position_.x += offset;
                return true;
            case GLFW_KEY_K:
                position_.y += offset;
                return true;
            case GLFW_KEY_J:
                position_.y -= offset;
                return true;
            case GLFW_KEY_EQUAL:
                scale_ *= 1.1f;
                return true;
            case GLFW_KEY_MINUS:
                scale_ /= 1.1f;
                return true;
            case GLFW_KEY_SPACE:
                reset_view();
                return true;
            default:
                break;
        }
    }
    return false;
}

void SceneCamera3d::switch_if_necessary(drag_machine_kind kind) {
    if (!drag_machine_ || drag_machine_->kind() != kind) {
        switch (kind) {
                case drag_machine_kind::rotator:
                drag_machine_ = std::make_unique<Rotator>(*this);
                break;
            default:
                break;
        }
    }
}

bool SceneCamera3d::handle_mouse_moved(float x, float y) {
    // update slices that is being hovered over
    y = -y;

    if (prev_y_ < -1.0) {
        prev_x_ = x;
        prev_y_ = y;
    }

    glm::vec2 delta(x - prev_x_, y - prev_y_);
    prev_x_ = x;
    prev_y_ = y;

    // TODO: fix for screen ratio ratio
    if (dragging_) {
        drag_machine_->on_drag(delta);
        return true;
    }

    return false;
}


void SceneCamera3d::tick(float time_elapsed) {
    (void)time_elapsed;
}

}  // namespace tomovis
