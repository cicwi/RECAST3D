#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/interface/window.hpp"
#include "graphics/material.hpp"
#include "graphics/scene_camera.hpp"
#include "graphics/scene_camera_3d.hpp"
#include "path.hpp"
#include "ticker.hpp"

namespace tomovis {

class MovieComponent;

class Animation {
  public:
    Animation(float at, float duration);
    virtual void update(float time) = 0;

  protected:
    float at_;
    float duration_;
};

float mix(float x, float y, float a);
glm::vec3 mix(glm::vec3 x, glm::vec3 y, float a);
Material mix(Material x, Material y, float a);

template <typename T>
class PropertyAnimation : public Animation {
  public:
    PropertyAnimation(float at, float duration, T begin, T end, T& target)
        : Animation(at, duration), begin_(begin), end_(end), target_(target) {}

    void update(float time) override {
        if (time < at_ || time > at_ + duration_) {
            return;
        }

        float alpha = (time - at_) / duration_;
        alpha = 3.0f * alpha * alpha - 2.0f * alpha * alpha * alpha;
        target_ = mix(begin_, end_, alpha);
    }

  private:
    T begin_;
    T end_;
    T& target_;
};

enum class motion_mode : int { natural_speed, constant_speed, custom_speed };

class MoveAlongPath : public Animation {
  public:
    MoveAlongPath(float at, float duration, Path3 const& path,
                  glm::vec3& target,
                  motion_mode mode = motion_mode::natural_speed);

    MoveAlongPath(std::vector<float> time_points, Path3 const& path,
                  glm::vec3& target);

    Path3 path() { return path_; }

    float time_to_param(float time);
    void update(float time) override;

  private:
    Path3 path_;
    std::vector<float> time_points_;
    glm::vec3& target_;
    motion_mode motion_mode_;
    float max_param_;
    Eigen::VectorXf arc_lengths_;
    int last_index_ = 0;
};

class MoveCameraAlongPath : public Animation {
  public:
    MoveCameraAlongPath(float at, float duration, Path3 const& path,
                        SceneCamera3d* camera,
                        motion_mode mode = motion_mode::natural_speed,
                        bool keep_vertical = true)
        : Animation(at, duration), camera_(camera),
          keep_vertical_(keep_vertical),
          position_animation_(
              MoveAlongPath(at, duration, path, camera->position(), mode)) {}

    MoveCameraAlongPath(std::vector<float> time_points, Path3 const& path,
                        SceneCamera3d* camera, bool keep_vertical = true)
        : Animation(time_points.empty() ? 0.0f : time_points[0],
                    time_points.empty()
                        ? 1.0f
                        : time_points[time_points.size() - 1] - time_points[0]),
          camera_(camera), keep_vertical_(keep_vertical),
          position_animation_(
              MoveAlongPath(time_points, path, camera->position())) {}

    void update(float time) override;

  private:
    SceneCamera3d* camera_;
    bool keep_vertical_;
    MoveAlongPath position_animation_;
};

class TriggerAnimation : public Animation {
  public:
    TriggerAnimation(float at, std::function<void()> callback)
        : Animation(at, 0.0f), callback_(callback) {}

    void update(float time) override {
        if (fired_ || time < at_) {
            return;
        }

        callback_();
        fired_ = true;
    }

  private:
    std::function<void()> callback_;
    bool fired_ = false;
};

class Script {
  public:
    template <typename TAnimation, typename... Ts>
    void animate(Ts&&... args) {
        animations.push_back(std::make_unique<TAnimation>(std::forward<Ts>(args)...));
    }

    std::function<void()> initial_scene;
    std::vector<std::unique_ptr<Animation>> animations;
    std::string name;
};

class Storyboard : public Window, public Ticker {
  public:
    Storyboard(MovieComponent* movie);
    ~Storyboard();

    void describe() override;
    void tick(float time_elapsed) override;

  private:
    void add_scripts_();
    void perform_script_();
    void reset_();

    bool running_ = false;
    float t_ = 0.0f;
    float animation_speed_ = 1.0f;

    std::vector<std::unique_ptr<Script>> scripts_;
    int current_script_ = 0;

    MovieComponent* movie_;
};

} // namespace tomovis
