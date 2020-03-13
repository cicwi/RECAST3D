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

    ImGui::SetNextWindowSizeConstraints(ImVec2(280, 500), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
    ImGui::Begin("Scene controls");
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);    // 2/3 of the space for widget and 1/3 for labels

    static char text_buffer[128];

    std::copy(scene.name().data(),
              scene.name().data() + scene.name().size() + 1, text_buffer);

    if (ImGui::InputText("name", text_buffer, 128)) {
        scene.set_name(std::string(text_buffer));
    }

    scene.object().describe();

    ImGui::End();
}

} // namespace tomovis
