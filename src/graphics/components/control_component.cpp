#include <iostream>

#include "tomop/tomop.hpp"
#include <imgui.h>

#include "graphics/components/control_component.hpp"
#include "graphics/scene_object.hpp"

namespace tomovis {

constexpr auto BUFFER_SIZE = 32u;

ControlComponent::ControlComponent(SceneObject& object, int scene_id)
    : object_(object), scene_id_(scene_id) {}

void ControlComponent::describe() {
    ImGui::Indent(16.0f);

    describe_benchmarks_();
    describe_parameters_();
    describe_trackers_();

    ImGui::Unindent(16.0f);
}

void ControlComponent::describe_parameters_() {
    if (bool_parameters_.empty() && float_parameters_.empty() &&
        enum_parameters_.empty()) {
        return;
    }
    if (ImGui::CollapsingHeader("parameters", nullptr,
                                ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto& [key, value] : bool_parameters_) {
            auto old = value;
            ImGui::Checkbox(key.c_str(), &value);
            if (value != old) {
                auto pkt =
                    tomop::ParameterBoolPacket(object_.scene_id(), key, value);
                object_.send(pkt);
            }
        }

        for (auto& [key, vb] : float_parameters_) {
            auto& [value, buffer] = vb;
            if (ImGui::InputText(key.c_str(), buffer.get(), BUFFER_SIZE,
                                 ImGuiInputTextFlags_CharsDecimal |
                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
                auto buffer_value = ::atof(buffer.get());
                value = buffer_value;
                auto pkt =
                    tomop::ParameterFloatPacket(object_.scene_id(), key, value);
                object_.send(pkt);
            }
        }

        for (auto& [key, vc] : enum_parameters_) {
            auto& [values, current] = vc;
            if (ImGui::BeginCombo(key.c_str(), current.c_str())) {
                for (auto& value : values) {
                    bool is_selected = (value == current);
                    if (ImGui::Selectable(value.c_str(), is_selected)) {
                        current = value;
                        auto pkt = tomop::ParameterEnumPacket(
                            object_.scene_id(), key, {current});
                        object_.send(pkt);
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
    }
}

void ControlComponent::describe_trackers_() {
    if (trackers_.empty()) {
        return;
    }

    // TODO only track last x timesteps

    if (ImGui::CollapsingHeader("trackers", nullptr,
                                ImGuiTreeNodeFlags_DefaultOpen)) {

        for (auto& [key, values] : trackers_) {
            ImGui::PlotLines(key.c_str(), values.data(), values.size(), 0,
                             nullptr, FLT_MAX, FLT_MAX, ImVec2(400.0f, 150.0f),
                             sizeof(float));
        }
    }
}

void ControlComponent::describe_benchmarks_() {
    if (benchmarks_.empty()) {
        return;
    }

    if (ImGui::CollapsingHeader("benchmarks", nullptr,
                                ImGuiTreeNodeFlags_DefaultOpen)) {

        // TODO add (rolling) average
        ImGui::Indent(16.0f);
        ImGui::Columns(2, "benchmarks");
        for (auto& [key, values] : benchmarks_) {
            ImGui::TextWrapped(key.c_str());
            ImGui::NextColumn();
            ImGui::TextWrapped(
                std::to_string(values[values.size() - 1]).c_str());
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::Unindent(16.0f);
    }
}

void ControlComponent::add_float_parameter(std::string name, float value) {
    auto cptr = std::unique_ptr<char[]>(new char[BUFFER_SIZE]);
    auto value_s = std::to_string(value);
    assert(value_s.size() < BUFFER_SIZE - 1);
    std::fill(cptr.get(), cptr.get() + BUFFER_SIZE, 0);
    std::copy(value_s.begin(), value_s.end(), cptr.get());
    float_parameters_[name] = {value, std::move(cptr)};
}
void ControlComponent::add_bool_parameter(std::string name, bool value) {
    bool_parameters_[name] = value;
}
void ControlComponent::add_enum_parameter(std::string name,
                                          std::vector<std::string> values) {
    if (values.empty()) {
        std::cout << "Refusing to add enum parameter with no options\n";
        return;
    }
    enum_parameters_[name] = {values, values[0]};
}

void ControlComponent::bench_result(std::string name, float value) {
    benchmarks_[name].push_back(value);
}

void ControlComponent::track_result(std::string name, float value) {
    trackers_[name].push_back(value);
}

} // namespace tomovis
