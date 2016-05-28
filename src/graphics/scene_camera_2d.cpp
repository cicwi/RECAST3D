#include "graphics/scene_camera_2d.hpp"

#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>


namespace tomovis {

SceneCamera2d::SceneCamera2d() : angle_(0.0f) {
    parameters_.push_back({"angle", 0.0f, 2.0f * M_PI, &angle_});
    parameters_.push_back({"x", -1.0f, 1.0f, &position_.x});
    parameters_.push_back({"y", -1.0f, 1.0f, &position_.y});
    parameters_.push_back({"scale", 0.0f, 1.0f, &scale_});
    scale_ = 1.0f;
}

glm::mat4 SceneCamera2d::matrix() {
    glm::mat4 camera_matrix;

    camera_matrix =
        glm::scale(glm::vec3(glm::vec2(scale_), 1.0f)) * camera_matrix;

    camera_matrix =
        glm::rotate(angle_, glm::vec3(0.0f, 0.0f, 1.0f)) * camera_matrix;

    camera_matrix =
        glm::translate(glm::vec3(position_, 0.0f)) * camera_matrix;

    return camera_matrix;
}

bool SceneCamera2d::handle_mouse_button(int button, bool down) {
    dragging_ = down;
    return true;
}

bool SceneCamera2d::handle_scroll(double offset) {
    scale_ += offset / 20.0;
    return true;
}

bool SceneCamera2d::handle_key(int key, bool down, int mods) {
    float offset = 0.05f;
    if (down) {
        switch(key) {
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
            default:
                break;
        }
    }
    return false;
}

bool SceneCamera2d::handle_mouse_moved(float x, float y) {
    if (prev_y_ < -1.0) {
        prev_x_ = x;
        prev_y_ = y;
    }

    // TODO: fix for screen ratio ratio
    if (dragging_) {
        auto dx = x - prev_x_;
        auto dy = y - prev_y_;

        position_.x += dx;
        position_.y -= dy;

        prev_x_ = x;
        prev_y_ = y;

        return true;
    }

    prev_x_ = x;
    prev_y_ = y;

    return false;
}

} // namespace tomovis
