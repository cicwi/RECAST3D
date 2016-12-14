#pragma once

#include <map>
#include <memory>

#include <glm/glm.hpp>

#include "graphics/render_target.hpp"
#include "input_handler.hpp"


namespace tomovis {

class Scene;

class SceneList : public RenderTarget, public InputHandler {
  public:
    SceneList();
    ~SceneList();

    int add_scene(std::string name, int id = 1, bool make_active = false);
    void delete_scene(int index);
    void set_active_scene(int index);
    int reserve_id();

    auto& scenes() { return scenes_; }
    Scene* active_scene() const { return active_scene_; }
    int active_scene_index() const { return active_scene_index_; }

    void render(glm::mat4 window_matrix) override;

    bool handle_mouse_button(int button, bool down) override;
    bool handle_scroll(double offset) override;
    bool handle_mouse_moved(float x, float y) override;
    bool handle_key(int key, bool down, int mods) override;

  private:
    std::map<int, std::unique_ptr<Scene>> scenes_;
    Scene* active_scene_ = nullptr;
    int active_scene_index_ = -1;
    int give_away_id_ = 0;
};

} // namespace tomovis
