#pragma once

#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include "graphics/scene_camera.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include "slice.hpp"

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
    SceneCamera3d(std::map<int, std::unique_ptr<slice>>& slices);

    glm::mat4 matrix() override;

    std::vector<parameter<float>>& parameters() override { return parameters_; }

    void reset_view();

    auto& view_matrix() { return view_matrix_; }
    auto& up() { return up_; }
    auto& right() { return right_; }

    void switch_if_necessary(drag_machine_kind kind);

    auto& dragged_slice() { return dragged_slice_; }
    auto& get_slices() { return slices_; }

    bool handle_mouse_button(int button, bool down) override;
    bool handle_scroll(double offset) override;
    bool handle_mouse_moved(float x, float y) override;
    bool handle_key(int key, bool down, int mods) override;

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

    glm::mat4 view_matrix_;
    // I think this should be a shared ptr or weak ptr, this is quite unsafe
    // but maybe it is okay because camera is part of the object so it should
    // get deconstructed before the object
    std::map<int, std::unique_ptr<slice>>& slices_;

    std::unique_ptr<CameraDragMachine> drag_machine_;
    slice* dragged_slice_ = nullptr;
};

class Rotator : public CameraDragMachine {
   public:
    using CameraDragMachine::CameraDragMachine;

    void on_drag(glm::vec2 delta) override {
        // rotate 'right' with angle '-dx' around 'up'
        camera_.view_matrix() =
            glm::rotate(3.0f * delta.x, camera_.up()) * camera_.view_matrix();
        camera_.view_matrix() = glm::rotate(3.0f * delta.y, camera_.right()) *
                                camera_.view_matrix();
    }

    drag_machine_kind kind() override { return drag_machine_kind::rotator; }
};

}  // namespace tomovis
