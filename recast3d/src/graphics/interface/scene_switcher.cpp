#include <functional>
#include <sstream>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "graphics/components/reconstruction_component.hpp"
#include "graphics/interface/scene_switcher.hpp"
#include "graphics/interface/window.hpp"
#include "scene.hpp"
#include "scene_list.hpp"

namespace tomovis {

SceneSwitcher::SceneSwitcher(SceneList& scenes) : scenes_(scenes) {
    reload_data_();
    current_item_ = 0;
}

SceneSwitcher::~SceneSwitcher() {}

void SceneSwitcher::reload_data_() {
    model_options_.clear();
    short_options_.clear();
    if (!fs::exists("../data"))
        return;
    for (auto entry : fs::directory_iterator("../data/")) {
        std::string name = entry.path().native();
        model_options_.push_back(name);

        int short_length = 15;
        std::string short_name = entry.path().filename();
        if ((int)short_name.size() < short_length) {
            short_options_.push_back(short_name);
        } else {
            short_options_.push_back(short_name.substr(0, short_length - 2) +
                                     "..");
        }
    }
}

void SceneSwitcher::describe() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Scenes")) {
            if (ImGui::MenuItem("Next scene", "ctrl + n")) {
                next_scene();
            }
            //            if (ImGui::MenuItem("Add scene", "ctrl + a")) {
            //                add_scene();
            //            }
            if (ImGui::MenuItem("Add scene (3D)", "ctrl + b")) {
                add_scene_3d();
            }
            if (ImGui::MenuItem("Delete scene", "ctrl + d")) {
                delete_scene();
            }
            if (ImGui::MenuItem("Load dataset", "ctrl + l")) {
                // show_dataset_modal();
            }

            if (scenes_.active_scene()) {

                ImGui::Separator();

                for (auto& scene : scenes_.scenes()) {
                    int index = scene.first;
                    if (ImGui::MenuItem(
                            scene.second.get()->name().c_str(), nullptr,
                            index == scenes_.active_scene_index())) {
                        scenes_.set_active_scene(index);
                    }
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (loading_dataset_) {
        ImGui::OpenPopup("Choose dataset");
        if (ImGui::BeginPopupModal("Choose dataset", NULL,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            static char str0[128] = "/";
            ImGui::InputText("location", str0, IM_ARRAYSIZE(str0));

            ImGui::Separator();

            const char* listbox_items[] = {"FleX-ray", "HDF5"};
            static int listbox_item_current = 0;
            ImGui::ListBox("Dataset type", &listbox_item_current, listbox_items,
                           IM_ARRAYSIZE(listbox_items));

            if (ImGui::Button("OK")) {
                add_dataset_scene(str0);
                loading_dataset_ = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel")) {
                loading_dataset_ = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}

bool SceneSwitcher::handle_key(int key, bool down, int mods) {
    if (down && key == GLFW_KEY_N && (mods & GLFW_MOD_CONTROL)) {
        next_scene();
        return true;
    }
    //    if (down && key == GLFW_KEY_A && (mods & GLFW_MOD_CONTROL)) {
    //        add_scene();
    //        return true;
    //    }
    if (down && key == GLFW_KEY_B && (mods & GLFW_MOD_CONTROL)) {
        add_scene_3d();
        return true;
    }
    if (down && key == GLFW_KEY_L && (mods & GLFW_MOD_CONTROL)) {
        // show_dataset_modal();
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
    if (active_scene_it == scenes_.scenes().end())
        active_scene_it = scenes_.scenes().begin();

    scenes_.set_active_scene((*active_scene_it).first);
}

void SceneSwitcher::add_scene() {
    std::cout << "2D scenes are disabled\n";
    //    std::stringstream ss;
    //    ss << "Scene #" << scenes_.scenes().size() + 1;
    //    scenes_.set_active_scene(scenes_.add_scene(ss.str()));
}

void SceneSwitcher::add_scene_3d() {
    std::stringstream ss;
    ss << "3D Scene #" << scenes_.scenes().size() + 1;
    scenes_.set_active_scene(scenes_.add_scene(ss.str(), -1, true, 3));
    auto& obj = scenes_.active_scene()->object();
    obj.add_component(
        std::make_unique<ReconstructionComponent>(obj, obj.scene_id()));
}

void SceneSwitcher::show_dataset_modal() { loading_dataset_ = true; }

void SceneSwitcher::add_dataset_scene(std::string location) {
    std::cout << "Add dataset: " << location << "\n";
    std::cout << "TODO...\n";
    // 1. start a new reconstruction server
    // Q: hold list of reconstruction threads?
    // Q: multiple reconsruction scenes cannot be held in GPU memory.. make
    // reconstruction scene exclusive?
    // Q: how do we delete a scene if it already exists
    // 1b. we want to kill the server when scene is deleted, add delete scene
    // package
    // 2. make a scene with recon component to the appropriate server
}

void SceneSwitcher::delete_scene() {
    if (!scenes_.active_scene())
        return;

    scenes_.delete_scene(scenes_.active_scene_index());
}

} // namespace tomovis
