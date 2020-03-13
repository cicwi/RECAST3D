#pragma once

#include <string>
#include <vector>

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
    void show_dataset_modal();

    void add_movie_scene(std::string model_file);
    void add_dataset_scene(std::string dataset_location);
    void delete_scene();

  private:
    SceneList& scenes_;
    bool adding_movie_ = false;
    bool loading_dataset_ = false;

    void reload_data_();

    int current_item_ = 0;
    std::vector<std::string> short_options_;
    std::vector<std::string> model_options_;
};

} // namespace tomovis
