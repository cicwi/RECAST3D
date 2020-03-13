#include <memory>

#include "scene.hpp"
#include "scene_list.hpp"

namespace tomovis {

SceneList::SceneList() {}

SceneList::~SceneList() {}

// TODO make thread safe
int SceneList::add_scene(std::string name, int id, bool make_active,
                         int dimension) {
    if (id == -1) {
        id = reserve_id();
    }

    scenes_[id] = std::make_unique<Scene>(name, dimension, id);
    if (make_active) {
        set_active_scene(id);
    }

    scenes_[id]->object().add_listener(this);

    return id;
}

// TODO make thread safe
int SceneList::reserve_id() { return give_away_id_++; }

void SceneList::delete_scene(int index) {
    scenes_.erase(scenes_.find(index));
    if (active_scene_index_ == index) {
        if (scenes_.empty()) {
            active_scene_index_ = -1;
            active_scene_ = nullptr;
        } else {
            active_scene_index_ = scenes_.begin()->first;
            active_scene_ = scenes_[active_scene_index_].get();
        }
    }
}

void SceneList::set_active_scene(int index) {
    active_scene_ = scenes_[index].get();
    active_scene_index_ = index;
}

void SceneList::render(glm::mat4 window_matrix) {
    if (active_scene_) {
        active_scene_->render(window_matrix);
    }
}

void SceneList::tick(float dt) {
    if (active_scene_) {
        return active_scene_->object().tick(dt);
    }
}

bool SceneList::handle_mouse_button(int button, bool down) {
    if (active_scene_) {
        return active_scene_->object().handle_mouse_button(button, down);
    }
    return false;
}

bool SceneList::handle_scroll(double offset) {
    if (active_scene_) {
        return active_scene_->object().handle_scroll(offset);
    }
    return false;
}

bool SceneList::handle_mouse_moved(float x, float y) {
    if (active_scene_) {
        return active_scene_->object().handle_mouse_moved(x, y);
    }
    return false;
}

bool SceneList::handle_key(int key, bool down, int mods) {
    if (active_scene_) {
        return active_scene_->object().handle_key(key, down, mods);
    }
    return false;
}

}  // namespace tomovis
