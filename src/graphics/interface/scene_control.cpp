#include <imgui.h>

#include "scene.hpp"
#include "scene_list.hpp"
#include "graphics/interface/scene_control.hpp"
#include "graphics/scene_camera.hpp"


namespace tomovis {

SceneControl::SceneControl(SceneList& scenes) : scenes_(scenes) {}

SceneControl::~SceneControl() {}

void SceneControl::describe() {
    if (!scenes_.active_scene())
        return;

    auto& scene = *scenes_.active_scene();

    ImGui::SetNextWindowSizeConstraint(ImVec2(280, 200), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
    ImGui::Begin("Scene controls");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);    // 2/3 of the space for widget and 1/3 for labels

    static char text_buffer[128];

    std::copy(scene.name().data(),
              scene.name().data() + scene.name().size() + 1, text_buffer);

    if (ImGui::InputText("name", text_buffer, 128)) {
        scene.set_name(std::string(text_buffer));
    }

    if(ImGui::CollapsingHeader("image")) {
        ImGui::SliderFloat("pixel size", &scene.object().size(), 0.1f, 1.0f);
    }

    if (ImGui::CollapsingHeader("camera")) {
        for (auto& p : scene.object().camera().parameters()) {
            ImGui::SliderFloat(p.name.c_str(), p.value, p.min_value,
                               p.max_value);
        }
    }

    ImGui::End();
}

} // namespace tomovis
