#include <iostream>
#include <limits>

#include <imgui.h>

#include "graphics/scene_camera_3d.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>

#include "path.hpp"

namespace tomovis {

SceneCamera3d::SceneCamera3d() { reset_view(); }

void SceneCamera3d::reset_view() {
    // explicitely set to identity
    position_ = glm::vec3(0.0f, 0.0f, 5.0f);
    up_ = glm::vec3(0.0f, 1.0f, 0.0f);
    right_ = glm::vec3(1.0f, 0.0f, 0.0f);
    rotation_ = glm::mat4(1.0f);

    SceneCamera3d::rotate(-0.25f * glm::pi<float>(), 0.0f);
}

void SceneCamera3d::set_look_at(glm::vec3 center) { center_ = center; }

void SceneCamera3d::set_position(glm::vec3 position) { position_ = position; }

void SceneCamera3d::set_right(glm::vec3 right) { right_ = right; }

void SceneCamera3d::set_up(glm::vec3 up) { up_ = up; }

void SceneCamera3d::rotate(float phi, float psi) {
    auto rotate_up = glm::rotate(phi, up_);
    auto rotate_right = glm::rotate(psi, right_);
    rotation_ = rotate_up * rotate_right * rotation_;
}

glm::mat4 SceneCamera3d::matrix() {
    glm::mat4 camera_matrix = glm::lookAt(position_, center_, up_);

    camera_matrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f) *
                    camera_matrix * rotation_;

    return camera_matrix;
}

bool SceneCamera3d::handle_mouse_button(int /* button */, bool down) {
    if (interaction_disabled_) {
        return false;
    }
    dragging_ = down;
    if (!dragging_) {
        drag_machine_ = nullptr;
    } else {
        switch_if_necessary(drag_machine_kind::rotator);
    }
    return true;
}

bool SceneCamera3d::handle_scroll(double offset) {
    if (interaction_disabled_) {
        return false;
    }

    position_ *= (1.0 - offset / 20.0);
    return true;
}

bool SceneCamera3d::handle_key(int key, bool down, int /* mods */) {
    if (interaction_disabled_) {
        return false;
    }

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
            drag_machine_ =
                std::make_unique<Rotator>(*this, prev_x_, prev_y_, instant_);
            break;
        default:
            break;
        }
    }
}

bool SceneCamera3d::handle_mouse_moved(float x, float y) {
    if (interaction_disabled_) {
        return false;
    }

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
        drag_machine_->on_drag({x, y}, delta);
        return true;
    }

    return false;
}

void SceneCamera3d::describe() {
    SceneCamera::describe();
    ImGui::Checkbox("Instant Camera", &instant_);

    if (ImGui::Button("camera xy")) {
        rotation_ = glm::mat4(1.0f);
        position_ = center_;
        position_.z += 5.0;
        up_ = glm::vec3(0.0f, 1.0f, 0.0f);
        right_ = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (ImGui::Button("camera yz")) {
        rotation_ = glm::mat4(1.0f);
        position_ = center_;
        position_.x += 5.0;
        up_ = glm::vec3(0.0f, 0.0f, 1.0f);
        right_ = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (ImGui::Button("camera xz")) {
        rotation_ = glm::mat4(1.0f);
        position_ = center_;
        position_.y -= 5.0;
        up_ = glm::vec3(0.0f, 0.0f, 1.0f);
        right_ = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (ImGui::Button("camera persp")) {
        rotation_ = glm::mat4(1.0f);
        position_ = center_;
        position_.z += 5.0;
        position_.y += 2.5;
        position_.x += 2.5;
        up_ = glm::vec3(0.0f, 1.0f, 0.0f);
        right_ = glm::vec3(1.0f, 0.0f, 0.0f);
    }
}

void SceneCamera3d::tick(float time_elapsed) {
    if (dragging_) {
        drag_machine_->tick(time_elapsed);
    }
}

} // namespace tomovis
