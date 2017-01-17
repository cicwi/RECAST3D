#include <iostream>
#include <limits>

#include "graphics/scene_camera_3d.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

namespace tomovis {

SceneCamera3d::SceneCamera3d(std::map<int, std::unique_ptr<slice>>& slices)
    : slices_(slices) {
    parameters_.push_back({"angle", 0.0f, 2.0f * M_PI, &angle_});
    parameters_.push_back({"x", -10.0f, 10.0f, &position_.x});
    parameters_.push_back({"y", -10.0f, 10.0f, &position_.y});
    parameters_.push_back({"z", -10.0f, 10.0f, &position_.z});
    parameters_.push_back({"scale", 0.0f, 1.0f, &scale_});
    parameters_.push_back(
        {"x of slice 1", -1.0f, 1.0f, &((slices_[0]->orientation)[0][0])});

    position_.z = -5.0f;

    up_.y = 1.0f;
    right_.x = 1.0f;

    reset_view();
    switch_if_necessary(drag_machine_kind::rotator);
}

void SceneCamera3d::reset_view() {
    // explicitely set to identity
    view_matrix_ = glm::mat4();
    view_matrix_ = glm::rotate(0.25f * glm::pi<float>(), up_) * view_matrix_;
    view_matrix_ =
        glm::rotate(-0.125f * glm::pi<float>(), right_) * view_matrix_;
}

glm::mat4 SceneCamera3d::matrix() {
    glm::mat4 camera_matrix = view_matrix_;

    camera_matrix =
        glm::lookAt(position_, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
        camera_matrix;

    camera_matrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f) *
                    camera_matrix;

    return camera_matrix;
}

bool SceneCamera3d::handle_mouse_button(int button, bool down) {
    dragging_ = down;
    if (!down)
        dragged_slice_ = nullptr;
    return true;
}

bool SceneCamera3d::handle_scroll(double offset) {
    scale_ += offset / 20.0;
    return true;
}

bool SceneCamera3d::handle_key(int key, bool down, int mods) {
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

bool SceneCamera3d::check_hovered(float x, float y) {
    auto intersection_point = [](glm::mat4 inv_matrix, glm::mat4 orientation,
                                 glm::vec2 point) -> std::pair<bool, float> {
        auto intersect_ray_plane = [](glm::vec3 origin, glm::vec3 direction,
                                      glm::vec3 base, glm::vec3 normal,
                                      float& distance) -> bool {
            auto alpha = glm::dot(normal, direction);
            if (glm::abs(alpha) > 0.001f) {
                distance = glm::dot((base - origin), normal) / alpha;
                if (distance >= 0.001f) return true;
            }
            return false;
        };

        // how do we want to do this
        // end points of plane/line?
        // first see where the end
        // points of the square end up
        // within the box.
        // in world space:
        auto o = orientation;
        auto axis1 = glm::vec3(o[0][0], o[0][1], o[0][2]);
        auto axis2 = glm::vec3(o[1][0], o[1][1], o[1][2]);
        auto base = glm::vec3(o[2][0], o[2][1], o[2][2]);
        base += 0.5f * (axis1 + axis2);
        auto normal = glm::normalize(glm::cross(axis1, axis2));
        float distance = -1.0f;

        auto from = inv_matrix * glm::vec4(point.x, point.y, -1.0f, 1.0f);
        from /= from[3];
        auto to = inv_matrix * glm::vec4(point.x, point.y, 1.0f, 1.0f);
        to /= to[3];
        auto direction = glm::normalize(glm::vec3(to) - glm::vec3(from));

        bool does_intersect = intersect_ray_plane(glm::vec3(from), direction,
                                                  base, normal, distance);

        // now check if the actual point is inside the plane
        auto intersection_point = glm::vec3(from) + direction * distance;
        intersection_point -= base;
        auto along_1 = glm::dot(intersection_point, glm::normalize(axis1));
        auto along_2 = glm::dot(intersection_point, glm::normalize(axis2));
        if (glm::abs(along_1) > 0.5f * glm::length(axis1) ||
            glm::abs(along_2) > 0.5f * glm::length(axis2))
            does_intersect = false;

        return std::make_pair(does_intersect, distance);
    };

    auto inv_matrix = glm::inverse(matrix());
    int best_slice_index = -1;
    float best_z = std::numeric_limits<float>::max();
    int slice_index = 0;
    for (auto& id_slice : slices_) {
        auto& slice = id_slice.second;
        slice->hovered = false;
        auto maybe_point =
            intersection_point(inv_matrix, slice->orientation, glm::vec2(x, y));
        if (maybe_point.first) {
            auto z = maybe_point.second;
            if (z < best_z) {
                best_z = z;
                best_slice_index = slice_index;
            }
        }
        ++slice_index;
    }
    if (best_slice_index >= 0) {
        slices_[best_slice_index]->hovered = true;
        switch_if_necessary(drag_machine_kind::mover);
    } else {
        switch_if_necessary(drag_machine_kind::rotator);
    }
}

void SceneCamera3d::switch_if_necessary(drag_machine_kind kind) {
    if (!drag_machine_ || drag_machine_->kind() != kind) {
        switch (kind) {
            case drag_machine_kind::mover:
                drag_machine_ = std::make_unique<SliceMover>(*this);
                break;
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
    } else {
        check_hovered(x, y);
    }

    return false;
}

}  // namespace tomovis
