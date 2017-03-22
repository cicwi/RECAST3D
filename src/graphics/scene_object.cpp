#include <memory>

#include <GL/gl3w.h>

#include "graphics/scene_object.hpp"
#include "graphics/shader_program.hpp"

#include "graphics/scene_camera.hpp"

namespace tomovis {

SceneObject::SceneObject(int scene_id) : scene_id_(scene_id) {}

SceneObject::~SceneObject() {
    glDeleteVertexArrays(1, &vao_handle_);
    glDeleteBuffers(1, &vbo_handle_);
}

bool SceneObject::handle_mouse_button(int button, bool down) {
    for (auto& id_and_comp : components_) {
        if (id_and_comp.second->handle_mouse_button(button, down)) {
            return true;
        }
    }

    if (camera_) {
        if (camera_->handle_mouse_button(button, down)) {
            return true;
        }
    }

    return false;
}

bool SceneObject::handle_scroll(double offset) {
    for (auto& id_and_comp : components_) {
        if (id_and_comp.second->handle_scroll(offset)) {
            return true;
        }
    }

    if (camera_) {
        if (camera_->handle_scroll(offset)) {
            return true;
        }
    }

    return false;
}

bool SceneObject::handle_mouse_moved(float x, float y) {
    for (auto& id_and_comp : components_) {
        if (id_and_comp.second->handle_mouse_moved(x, y)) {
            return true;
        }
    }

    if (camera_) {
        if (camera_->handle_mouse_moved(x, y)) {
            return true;
        }
    }

    return false;
}

bool SceneObject::handle_key(int key, bool down, int mods) {
    for (auto& id_and_comp : components_) {
        if (id_and_comp.second->handle_key(key, down, mods)) {
            return true;
        }
    }

    if (camera_) {
        if (camera_->handle_key(key, down, mods)) {
            return true;
        }
    }


    return false;
}

}  // namespace tomovis
