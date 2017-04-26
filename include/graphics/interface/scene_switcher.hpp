#pragma once

#include "graphics/interface/window.hpp"
#include "input_handler.hpp"


namespace tomovis {

class SceneList;

class SceneSwitcher : public Window, public InputHandler {
  public:
    SceneSwitcher(SceneList& scenes);
    ~SceneSwitcher();

    void describe() override;

    bool handle_key(int key,  bool down, int mods) override;
    int priority() const override { return 2; }

    void next_scene();
    void add_scene();
    void add_scene_3d();
    void show_movie_modal();
    void add_movie_scene(std::string model_file);
    void delete_scene();

  private:
    SceneList& scenes_;
    bool adding_movie_ = false;
};

} // namespace tomovis
