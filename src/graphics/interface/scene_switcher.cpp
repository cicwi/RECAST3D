#include <sstream>

#include <imgui.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "graphics/interface/scene_switcher.hpp"
#include "graphics/interface/window.hpp"
#include "scene_list.hpp"
#include "scene.hpp"


namespace tomovis {

SceneSwitcher::SceneSwitcher(SceneList& scenes) : scenes_(scenes) {}

SceneSwitcher::~SceneSwitcher() {}

void SceneSwitcher::describe() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Scenes")) {
            if (ImGui::MenuItem("Next scene", "ctrl + n"))
                next_scene();
            if (ImGui::MenuItem("Add scene", "ctrl + a"))
                add_scene();
            if (ImGui::MenuItem("Delete scene", "ctrl + d"))
                delete_scene();

            if (scenes_.active_scene()) {

                ImGui::Separator();

                for (auto& scene : scenes_.scenes()) {
                    int index = scene.first;
                    if (ImGui::MenuItem(scene.second.get()->name().c_str(), nullptr,
                                        index == scenes_.active_scene_index())) {
                        scenes_.set_active_scene(index);
                    }
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


bool SceneSwitcher::handle_key(int key, bool down, int mods) {
    if (down && key == GLFW_KEY_N && (mods & GLFW_MOD_CONTROL)) {
        next_scene();
        return true;
    }
    if (down && key == GLFW_KEY_A && (mods & GLFW_MOD_CONTROL)) {
        add_scene();
        return true;
    }
    if (down && key == GLFW_KEY_D && (mods & GLFW_MOD_CONTROL)) {
        delete_scene();
        return true;
    }
    return false;
}

void SceneSwitcher::next_scene() {
    if (scenes_.scenes().empty())
        return;

    auto active_scene_it = scenes_.scenes().find(scenes_.active_scene_index());
    ++active_scene_it;
    if(active_scene_it == scenes_.scenes().end())
        active_scene_it = scenes_.scenes().begin();

    scenes_.set_active_scene((*active_scene_it).first);
}

void SceneSwitcher::add_scene() {
    std::stringstream ss;
    ss << "Scene #" << scenes_.scenes().size() + 1;
    scenes_.set_active_scene(scenes_.add_scene(ss.str()));
}

void SceneSwitcher::delete_scene() {
    if (!scenes_.active_scene())
        return;

    scenes_.delete_scene(scenes_.active_scene_index());
}

} // namespace tomovis
