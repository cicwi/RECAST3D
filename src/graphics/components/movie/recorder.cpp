#include "graphics/components/movie/recorder.hpp"
#include <imgui.h>

namespace tomovis {

Recorder::Recorder() {}
Recorder::~Recorder() {}

void Recorder::describe() {
    if (ImGui::Button("rec")) {
        recording_ = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("pause")) {
        recording_ = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("stop")) {
        recording_ = false;
    }
}

void Recorder::tick(float time_elapsed) {
    (void)time_elapsed;
    if (!recording_) {
        return;
    }
}

} // namespace tomovis
