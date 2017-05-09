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

float MoveAlongPath::time_to_param(float time) {
    if (time < at_ || time > at_ + duration_) {
        return -1.0f;
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
        return -1.0f;
    }
    return alpha;
}

void MoveAlongPath::update(float time) {
    if (time < at_ || time > at_ + duration_) {
        return;
    }
    auto point = path_(time_to_param(time));
    target_[0] = point(0);
    target_[1] = point(1);
    target_[2] = point(2);
}

void MoveCameraAlongPath::update(float time) {
    position_animation_.update(time);
    if (keep_vertical_) {
        return;
    }

    // Determine direction into which the camera is looking
    Eigen::RowVector3f look_at_dir;
    look_at_dir << camera_->look_at()[0] - camera_->position()[0],
        camera_->look_at()[1] - camera_->position()[1],
        camera_->look_at()[2] - camera_->position()[2];
    auto look_at_dir_norm = look_at_dir.norm();
    if (look_at_dir_norm < 1e-6) {
        return;
    }
    look_at_dir /= look_at_dir_norm;

    // Compute the new right vector as projection of the tangent onto the normal
    // plane fo the look-at direction. Do it such that up remains up.
    Eigen::RowVector3f tang = position_animation_.path().unit_tangent(
        position_animation_.time_to_param(time));
    std::cerr << "tangent " << tang << std::endl;
    Eigen::RowVector3f right = tang - tang.dot(look_at_dir) * look_at_dir;
    if (right.norm() < 1e-6) {
        return;
    }
    // TODO: something broken here, scene goes all crazy
    Eigen::RowVector3f up = right.cross(look_at_dir);
    if (up(1) <= 0) {
        up *= -1.0f;
    }
    std::cerr << "up " << up << std::endl;
    std::cerr << "right " << right << std::endl;
    camera_->set_up(glm::vec3(up(0), up(1), up(2)));
    return;
}

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

    /* 
    Example use of path animation:
    
    std::vector<glm::vec3> path_points = {
        {0.0f, 0.0f, 10.0f}, {5.0f, -5.0f, 0.0f}, {0.0f, 0.0f, -10.0f},
        {-5.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 10.0f}, {0.0f, 2.0f, 5.0f},
        {2.0f, 2.0f, 5.0f}};
    BdryConds3 bcs({bdry_cond::natural, bdry_cond::zero, bdry_cond::natural});
    Path3 path(path_points, bcs);
    std::vector<float> time_pts = {0.0f, 0.5f, 1.5f, 2.5f, 3.0f, 5.0f, 7.0f};
    animations_.push_back(std::make_unique<MoveCameraAlongPath>(
        time_pts, path, (SceneCamera3d*)&movie_->object().camera(), true));

    std::vector<glm::vec3> look_at_points = {
        {0.0f, 0.0f, 0.0f},   {0.0f, -2.0f, -2.0f}, {0.0f, 0.0f, 0.0f},
        {0.0f, -2.0f, -2.0f}, {0.0f, 0.0f, 0.0f},   {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}};
    Path3 look_at_path(look_at_points);

    // TODO: add to MoveCameraAlongPath instead of making an additional property
    // animation?
    animations_.push_back(std::make_unique<MoveAlongPath>(
        time_pts, look_at_path, movie_->object().camera().look_at())); */
    
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
