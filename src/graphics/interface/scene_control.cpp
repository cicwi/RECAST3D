#include <imgui.h>

#include "scene.hpp"
#include "graphics/interface/scene_control.hpp"
#include "graphics/scene_camera.hpp"


namespace tomovis {

SceneControl::SceneControl(Scene& scene) : scene_(scene) {}

SceneControl::~SceneControl() {}

void SceneControl::describe() {
    ImGui::SetNextWindowSizeConstraint(ImVec2(280, 200), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
    ImGui::Begin("Scene controls");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);    // 2/3 of the space for widget and 1/3 for labels
    if(ImGui::CollapsingHeader("image")) {
        ImGui::SliderFloat("pixel size", &scene_.object().size(), 0.1f, 1.0f);
    }

    if (ImGui::CollapsingHeader("camera")) {
        for (auto& p : scene_.object().camera().parameters()) {
            ImGui::SliderFloat(p.name.c_str(), p.value, p.min_value,
                               p.max_value);
        }
    }

    ImGui::End();
}

} // namespace tomovis
