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

void Storyboard::script_() {
    animations_.clear();

    Material bland;
    Material bland_transparent;
    bland_transparent.opacity = 0.1f;

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        0.0f, 2.0f, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 2.0f, 5.0f),
        movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        2.0f, 2.0f, glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(2.0f, 2.0f, 5.0f),
        movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        6.0f, 2.0f, glm::vec3(0.0f, 0.0f, 6.0f), glm::vec3(0.0f, 0.0f, 3.0f),
        movie_->projection()->source()));

    size_t i = 0;
    for (auto& mesh : movie_->model()->meshes()) {
        mesh->material() = bland;
        animations_.push_back(std::make_unique<PropertyAnimation<Material>>(
            4.0f, 2.0f, bland, mesh->mesh_material(), mesh->material()));
        if (i != 3) {
            animations_.push_back(std::make_unique<PropertyAnimation<Material>>(
                6.0f, 0.5f, mesh->mesh_material(), bland_transparent, mesh->material()));
        }
        ++i;
    }

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        8.0f, 2.0f, glm::vec3(2.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 2.0f),
        movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        8.0f, 2.0f, movie_->object().camera().look_at(), glm::vec3(0.0f, 0.5f, 0.0f),
        movie_->object().camera().look_at()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        12.0f, 2.0f, glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, -5.0f),
        movie_->object().camera().position()));
}

void Storyboard::describe() {
    ImGui::Text("Storyboard");

    if (ImGui::Button("run")) {
        movie_->object().camera().toggle_interaction();
        running_ = true;
        script_();
    }

    ImGui::SameLine();

    if (ImGui::Button("reset")) {
        movie_->object().camera().toggle_interaction();
        running_ = false;
        t_ = 0.0f;
    }
}

void Storyboard::tick(float time_elapsed) {
    if (!running_) {
        return;
    }

    t_ += time_elapsed;

    for (auto& anim : animations_) {
        anim->update(t_);
    }
}

} // namespace tomovis
