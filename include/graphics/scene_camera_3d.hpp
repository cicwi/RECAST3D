#pragma once

#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include "graphics/scene_camera.hpp"

#include <glm/gtx/rotate_vector.hpp>

namespace tomovis {

class SceneCamera3d;

enum class drag_machine_kind : int {
    none,
    rotator,
};

class CameraDragMachine {
   public:
    CameraDragMachine(SceneCamera3d& camera) : camera_(camera) {}

    virtual void on_drag(glm::vec2 delta) = 0;
    virtual drag_machine_kind kind() = 0;

   protected:
    SceneCamera3d& camera_;
};

class SceneCamera3d : public SceneCamera {
   public:
    SceneCamera3d();

    glm::mat4 matrix() override;

    std::vector<parameter<float>>& parameters() override { return parameters_; }

    void reset_view();

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

    glm::vec3& position() override { return position_; }
    glm::vec3& look_at() override { return center_; }

    void rotate(float phi, float psi);

   private:
    glm::vec3 position_;
    std::vector<parameter<float>> parameters_;

    float angle_ = 0.0f;
    float scale_ = 0.5f;

    float prev_x_ = -1.1f;
    float prev_y_ = -1.1f;
    glm::vec2 delta_;

    bool dragging_ = false;

    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 center_;

    std::unique_ptr<CameraDragMachine> drag_machine_;
};

class Rotator : public CameraDragMachine {
   public:
    using CameraDragMachine::CameraDragMachine;

    void on_drag(glm::vec2 delta) override {
        camera_.rotate(3.0f * delta.x, -3.0f * delta.y);
    }

    drag_machine_kind kind() override { return drag_machine_kind::rotator; }
};

}  // namespace tomovis
