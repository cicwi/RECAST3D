#include <memory>

#include "scene_list.hpp"
#include "scene.hpp"
#include "graphics/scene_camera.hpp"


namespace tomovis {

SceneList::SceneList() {}

SceneList::~SceneList() {}

int SceneList::add_scene(std::string name) {
    scenes_.push_back(std::make_unique<Scene>(name));
    if (!active_scene_) {
        active_scene_ = scenes_.back().get();
        active_scene_index_ = scenes_.size() - 1;
    }
    return scenes_.size() - 1;
}

void SceneList::delete_scene(int index) {
    scenes_.erase(scenes_.begin() + index);
    int scene_count = scenes_.size();
    if (active_scene_index_ > index) {
        active_scene_index_ -= 1;
    } else if (active_scene_index_ == scene_count) {
        active_scene_index_ -= 1;
    } if (scenes_.empty()) {
        active_scene_ = nullptr;
    } else {
        active_scene_ = scenes_[active_scene_index_].get();
    }
}

void SceneList::set_active_scene(int index) {
    active_scene_ = scenes_[index].get();
    active_scene_index_ = index;
}

void SceneList::render(glm::mat4 window_matrix) {
    if (active_scene_)
        active_scene_->render(window_matrix);
}

bool SceneList::handle_mouse_button(int button, bool down) {
    if (active_scene_)
        return active_scene_->object().camera().handle_mouse_button(button, down);
    return false;
}

bool SceneList::handle_scroll(double offset) {
    if (active_scene_)
        return active_scene_->object().camera().handle_scroll(offset);
    return false;
}

bool SceneList::handle_mouse_moved(float x, float y) {
    if (active_scene_)
        return active_scene_->object().camera().handle_mouse_moved(x, y);
    return false;
}

bool SceneList::handle_key(int key, bool down, int mods) {
    if (active_scene_)
        return active_scene_->object().camera().handle_key(key, down, mods);
    return false;
}

} // namespace tomovis
