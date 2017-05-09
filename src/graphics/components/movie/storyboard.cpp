#include <iostream>

#include "graphics/components/movie/storyboard.hpp"
#include "graphics/components/movie_component.hpp"
#include "graphics/scene_camera_3d.hpp"

#include "graphics/mesh.hpp"

#include <imgui.h>

namespace tomovis {

Animation::Animation(float at, float duration) : at_(at), duration_(duration) {}

Storyboard::Storyboard(MovieComponent* movie) : movie_(movie) { script_(); }

Storyboard::~Storyboard() {}

float mix(float x, float y, float a) { return (1.0f - a) * x + a * y; }

glm::vec3 mix(glm::vec3 x, glm::vec3 y, float a) { return glm::mix(x, y, a); }

Material mix(Material x, Material y, float a) {
    Material z;
    z.ambient_color = mix(x.ambient_color, y.ambient_color, a);
    z.diffuse_color = mix(x.diffuse_color, y.diffuse_color, a);
    z.specular_color = mix(x.specular_color, y.specular_color, a);
    z.opacity = mix(x.opacity, y.opacity, a);
    z.shininess = mix(x.shininess, y.shininess, a);
    return z;
}

void Storyboard::initial_scene_() {
    Material bland;
    for (auto& mesh : movie_->model()->meshes()) {
        mesh->material() = bland;
        mesh->reset_internal_time();
        mesh->set_visible(true);
    }

    movie_->object().camera().reset_view();
    movie_->object().camera().position() = glm::vec3(0.0f, 0.0f, 10.0f);
    movie_->projection()->source() = glm::vec3(0.0f, 0.0f, 6.0f);
    movie_->model()->rotations_per_second(0.06125f);
    movie_->model()->phi() = 0.0f;
}

void Storyboard::script_() {
    // Set initial scene
    animations_.clear();

    Material bland;
    Material bland_transparent;
    bland_transparent.opacity = 0.1f;

    initial_scene_();

    // Phase 0
    // Introduce scene
    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        0.0f, 8.0f, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 2.0f, 5.0f),
        movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        8.0f, 8.0f, glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(2.0f, 2.0f, 5.0f),
        movie_->object().camera().position()));

    float phase_1 = 16.0f;
    // Phase 1
    animations_.push_back(std::make_unique<TriggerAnimation>(
        phase_1, [&]() { movie_->model()->toggle_rotate(); }));

    animations_.push_back(std::make_unique<TriggerAnimation>(
        phase_1 + 16.0f, [&]() { movie_->model()->toggle_rotate(); }));

    float phase_2 = 60.0f;

    // Phase 2
    size_t i = 0;
    for (auto& mesh : movie_->model()->meshes()) {
        animations_.push_back(std::make_unique<PropertyAnimation<Material>>(
            phase_2, 3.0f, bland, mesh->mesh_material(), mesh->material()));
        if (i != 10) {
            animations_.push_back(std::make_unique<PropertyAnimation<Material>>(
                phase_2 + 20.0f, 3.0f, mesh->mesh_material(), bland_transparent,
                mesh->material()));
        }
        ++i;
    }

    animations_.push_back(std::make_unique<TriggerAnimation>(
        phase_2, [&]() { movie_->model()->toggle_rotate(); }));

    animations_.push_back(std::make_unique<TriggerAnimation>(
        phase_2 + 16.0f, [&]() { movie_->model()->toggle_rotate(); }));

    // Phase 3
    float phase_3 = 110.0f;
    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        phase_3, 4.0f, glm::vec3(2.0f, 2.0f, 5.0f), glm::vec3(8.0f, 0.0f, 2.0f),
        movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        phase_3 + 5.0f, 4.0f, glm::vec3(0.0f, 0.0f, 6.0f),
        glm::vec3(0.0f, 0.0f, 1.5f), movie_->projection()->source()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        phase_3 + 15.0f, 4.0f, glm::vec3(8.0f, 0.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 3.5f), movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        phase_3 + 15.0f, 4.0f, movie_->object().camera().look_at(),
        glm::vec3(0.0f, -0.5f, 0.0f), movie_->object().camera().look_at()));

    size_t j = 0;
    for (auto& mesh : movie_->model()->meshes()) {
        if (j != 10) {
            animations_.push_back(std::make_unique<TriggerAnimation>(
                phase_3 + 15.0f, [&]() { mesh->set_visible(false); }));
        }
        ++j;
    }

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        phase_3 + 25.0f, 4.0f, glm::vec3(0.0f, -0.5f, 0.0f),
        glm::vec3(0.3f, 0.3f, -2.5f), movie_->object().camera().look_at()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        phase_3 + 25.0f, 4.0f, glm::vec3(0.0f, 0.0f, 3.5f),
        glm::vec3(0.3f, 0.3f, 2.0f), movie_->object().camera().position()));
}

void Storyboard::describe() {
    ImGui::Text("Storyboard");

    if (ImGui::Button("run") && !running_) {
        movie_->object().camera().toggle_interaction();
        running_ = true;
        script_();
    }

    ImGui::SameLine();

    if (ImGui::Button("reset")) {
        movie_->object().camera().toggle_interaction();
        running_ = false;
        t_ = 0.0f;
        initial_scene_();
    }

    ImGui::SliderFloat("Animation speed", &animation_speed_, 1.0f, 10.0f);
    movie_->model()->rotations_per_second(animation_speed_ * 0.06125f);
}

void Storyboard::tick(float time_elapsed) {
    if (!running_) {
        return;
    }

    t_ += animation_speed_ * time_elapsed;

    for (auto& anim : animations_) {
        anim->update(t_);
    }
}

} // namespace tomovis
