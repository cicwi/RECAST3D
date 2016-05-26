#include <imgui.h>

#include "graphics/interface/scene_control.hpp"
#include "scene.hpp"


namespace tomovis {

SceneControl::SceneControl(Scene& scene) : scene_(scene) {}

SceneControl::~SceneControl() {}

void SceneControl::describe() {
    static float f = 0.0f;
    int show_another_window = 0;
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("block_size", &scene_.object().size(), 0.0f, 1.0f);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
}

} // namespace tomovis
