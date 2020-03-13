#pragma once

#include <map>
#include <memory>
#include <vector>

#include "graphics/scene_camera.hpp"
#include "path.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace tomovis {

class SceneCamera3d;

enum class drag_machine_kind : int {
    none,
    rotator,
};

class CameraDragMachine : public Ticker {
  public:
    CameraDragMachine(SceneCamera3d& camera) : camera_(camera) {}

    virtual void on_drag(glm::vec2 cur, glm::vec2 delta) = 0;
    virtual drag_machine_kind kind() = 0;
    void tick(float) override {}

  protected:
    SceneCamera3d& camera_;
};

class SceneCamera3d : public SceneCamera {
  public:
    SceneCamera3d();

    glm::mat4 matrix() override;

    void reset_view() override;

    auto& up() { return up_; }
    auto& right() { return right_; }

    void switch_if_necessary(drag_machine_kind kind);

    bool handle_mouse_button(int button, bool down) override;
    bool handle_scroll(double offset) override;
    bool handle_mouse_moved(float x, float y) override;
    bool handle_key(int key, bool down, int mods) override;

    void tick(float time_elapsed) override;

    void set_look_at(glm::vec3 center) override;
    void set_position(glm::vec3 position);
    void set_right(glm::vec3 right);
    void set_up(glm::vec3 up);

    glm::vec3& position() override { return position_; }
    glm::vec3& look_at() override { return center_; }

    void rotate(float phi, float psi);
    void describe() override;

  private:
    glm::vec3 position_;

    float angle_ = 0.0f;
    float scale_ = 0.5f;

    float prev_x_ = -1.1f;
    float prev_y_ = -1.1f;
    glm::vec2 delta_;

    bool dragging_ = false;
    bool instant_ = true;

    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 center_;
    glm::mat4 rotation_;

    float total_time_ = 0.0f;
    bool toggled_ = false;

    std::unique_ptr<CameraDragMachine> drag_machine_;
};

class Rotator : public CameraDragMachine {
  public:
    using CameraDragMachine::CameraDragMachine;

    Rotator(SceneCamera3d& camera, float x, float y, bool instant = true)
        : Rotator(camera) {
        x_ = x;
        y_ = y;
        cx_ = x;
        cy_ = y;
        instant_ = instant;
    }

    void on_drag(glm::vec2 cur, glm::vec2 delta) override {
        if (instant_) {
            camera_.rotate(3.0f * delta.x, -3.0f * delta.y);
        } else {
            cx_ = cur.x;
            cy_ = cur.y;
        }
    }

    void tick(float time_elapsed) override {
        if (instant_) {
            return;
        }
        camera_.rotate(10.0f * time_elapsed * (cx_ - x_),
                       -10.0f * time_elapsed * (cy_ - y_));
    }

    drag_machine_kind kind() override { return drag_machine_kind::rotator; }

  private:
    float x_;
    float y_;
    float cx_;
    float cy_;
    bool instant_ = true;
};

} // namespace tomovis
