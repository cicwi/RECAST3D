#include <iostream>

#include "graphics/components/movie/storyboard.hpp"
#include "graphics/components/movie_component.hpp"
#include "graphics/scene_camera_3d.hpp"

#include "graphics/mesh.hpp"

#include <eigen3/Eigen/Dense>
#include <imgui.h>

#include "path.hpp"

namespace tomovis {

Animation::Animation(float at, float duration) : at_(at), duration_(duration) {}

MoveAlongPath::MoveAlongPath(std::vector<float> time_points, Path3 const& path,
                             glm::vec3& target)
    : Animation(time_points.empty() ? 0.0f : time_points[0],
                time_points.empty()
                    ? 1.0f
                    : time_points[time_points.size() - 1] - time_points[0]),
      path_(path), time_points_(time_points), target_(target),
      motion_mode_(motion_mode::custom_speed) {
    assert(time_points.size() == static_cast<size_t>(path_.num_nodes()));
    // Make sure time points are increasing
    for (size_t i = 0; i < time_points.size(); ++i) {
        if (i < time_points.size() - 1) {
            assert(time_points[i + 1] - time_points[i] > 0.0f);
        }
    }
}

MoveAlongPath::MoveAlongPath(float at, float duration, Path3 const& path,
                             glm::vec3& target, motion_mode mode)
    : Animation(at, duration), path_(path), target_(target),
      motion_mode_(mode) {
    switch (mode) {
    case motion_mode::constant_speed: {
        // Approximate the path length with 100 samples per piece
        arc_lengths_ = path_.arc_length_lin_approx(100 * path_.num_pieces());
        arc_lengths_(arc_lengths_.size() - 1);
        break;
    }
    default:
        break;
    }
    for (int i = 0; i < path_.num_pieces() - 1; ++i) {
        time_points_.push_back(at + i * duration / (path_.num_pieces()));
    }
    time_points_.push_back(at + duration);
}

void MoveAlongPath::update(float time) {
    if (time < at_ || time > at_ + duration_) {
        return;
    }

    float alpha = 0.0f;
    switch (motion_mode_) {
    case motion_mode::natural_speed: {
        // Rescale time to `[0, N]`, where `N` is the number of pieces
        alpha = (time - at_) / duration_ * path_.num_pieces();
        break;
    }
    case motion_mode::constant_speed: {
        // Rescale time to `[0, total_len]`, where `total_len` is the total arc
        // length of the path. This yields the target arc length.
        float alen =
            (time - at_) / duration_ * arc_lengths_(arc_lengths_.size() - 1);
        // Get the interpolating index and weight and use it on the equidistant
        // natural parameters to get the parameter `alpha` that produces the
        // desired arc length. We use the fact that time is always increasing,
        // so we don't have to search before the last interpolating index.
        float alen_left = arc_lengths_(0), alen_right = arc_lengths_(1);
        int i;
        for (i = last_index_; i < arc_lengths_.size(); ++i) {
            if (arc_lengths_(i) > alen) {
                alen_left = arc_lengths_(i - 1); // i - 1 cannot go negative
                alen_right = arc_lengths_(i);
                last_index_ = i - 1;
                break;
            }
        }
        if (i == arc_lengths_.size()) {
            alpha = max_param_;
        } else {
            float s = (alen - alen_left) / (alen_right - alen_left);
            alpha =
                (last_index_ + s) *
                (static_cast<float>(path_.num_pieces()) / arc_lengths_.size());
        }
        break;
    }
    case motion_mode::custom_speed: {
        // Find out between which time points we are. Take the lower index of
        // the two plus the relative position within this interval as the
        // parameter.
        size_t i;
        float t_left = time_points_[0], t_right = time_points_[1];
        for (i = last_index_; i < time_points_.size(); ++i) {
            if (time_points_[i] > time) {
                t_left = time_points_[i - 1]; // i - 1 cannot underflow
                t_right = time_points_[i];
                last_index_ = i - 1;
                break;
            }
        }
        if (i == time_points_.size()) {
            alpha = static_cast<float>(path_.num_pieces());
        } else {
            float s = (time - t_left) / (t_right - t_left);
            alpha = last_index_ + s;
        }
        break;
    }
    default:
        return;
    }
    auto point = path_(alpha);
    target_[0] = point(0);
    target_[1] = point(1);
    target_[2] = point(2);
}

Storyboard::Storyboard(MovieComponent* movie) : movie_(movie) { script_(); }

Storyboard::~Storyboard() {}

float mix(float x, float y, float a) { return (1.0f - a) * x + a * y; }

glm::vec3 mix(glm::vec3 x, glm::vec3 y, float a) {
    return glm::mix(x, y, 3.0f * a * a - 2.0f * a * a * a);
}

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
    }

    movie_->object().camera().reset_view();
    movie_->object().camera().position() = glm::vec3(0.0f, 0.0f, 10.0f);
    movie_->projection()->source() = glm::vec3(0.0f, 0.0f, 6.0f);
    movie_->model()->rotations_per_second(0.125f);
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

    Eigen::Matrix<float, 7, 3> path_points;
    path_points.row(0) << 0.0f, 0.0f, 10.0f;
    path_points.row(1) << 5.0f, -5.0f, 0.0f;
    path_points.row(2) << 0.0f, 0.0f, -10.0f;
    path_points.row(3) << -5.0f, 5.0f, 0.0f;
    path_points.row(4) << 0.0f, 0.0f, 10.0f;
    path_points.row(5) << 0.0f, 2.0f, 5.0f;
    path_points.row(6) << 2.0f, 2.0f, 5.0f;
    Path3 path(path_points);
    std::vector<float> time_pts = {0.0f, 0.5f, 1.5f, 2.5f, 3.0f, 5.0f, 7.0f};
    animations_.push_back(std::make_unique<MoveAlongPath>(
        time_pts, path, movie_->object().camera().position()));

    /*    animations_.push_back(std::make_unique<MoveAlongPath>(
            0.0f, 6.0f, path, movie_->object().camera().position(),
            motion_mode::natural_speed));*/

    /*    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
            0.0f, 2.0f, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 2.0f,
       5.0f), movie_->object().camera().position()));

        animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
            2.0f, 2.0f, glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(2.0f, 2.0f,
       5.0f), movie_->object().camera().position()));*/

    /** Phase 1 */

    animations_.push_back(std::make_unique<TriggerAnimation>(
        4.0f, [&]() { movie_->model()->toggle_rotate(); }));

    animations_.push_back(std::make_unique<TriggerAnimation>(
        8.0f, [&]() { movie_->model()->toggle_rotate(); }));

    size_t i = 0;
    for (auto& mesh : movie_->model()->meshes()) {
        animations_.push_back(std::make_unique<PropertyAnimation<Material>>(
            10.0f, 2.0f, bland, mesh->mesh_material(), mesh->material()));
        if (i != 10) {
            animations_.push_back(std::make_unique<PropertyAnimation<Material>>(
                14.0f, 1.0f, mesh->mesh_material(), bland_transparent,
                mesh->material()));
        }
        ++i;
    }

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        15.0f, 2.0f, glm::vec3(2.0f, 2.0f, 5.0f), glm::vec3(8.0f, 0.0f, 0.0f),
        movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        17.0f, 2.0f, glm::vec3(0.0f, 0.0f, 6.0f), glm::vec3(0.0f, 0.0f, 1.5f),
        movie_->projection()->source()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        21.0f, 2.0f, glm::vec3(8.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.5f, 2.0f),
        movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        21.0f, 2.0f, movie_->object().camera().look_at(),
        glm::vec3(0.0f, 0.5f, 0.0f), movie_->object().camera().look_at()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        26.0f, 2.0f, glm::vec3(0.0f, 0.5f, 2.0f), glm::vec3(0.0f, 0.0f, -8.0f),
        movie_->object().camera().position()));

    animations_.push_back(std::make_unique<PropertyAnimation<glm::vec3>>(
        26.0f, 2.0f, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
        movie_->object().camera().look_at()));
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
        initial_scene_();
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
