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
    mover,
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

    bool handle_mouse_button(int button, bool down) override;
    bool handle_scroll(double offset) override;
    bool handle_mouse_moved(float x, float y) override;
    bool handle_key(int key, bool down, int mods) override;

    void reset_view();

    auto& view_matrix() { return view_matrix_; }
    auto& up() { return up_; }
    auto& right() { return right_; }

    bool check_hovered(float x, float y);
    void switch_if_necessary(drag_machine_kind kind);

    auto& dragged_slice() { return dragged_slice_; }
    auto& get_slices() { return slices_; }

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

class SliceMover : public CameraDragMachine {
   public:
    using CameraDragMachine::CameraDragMachine;

    void on_drag(glm::vec2 delta) override {
        // 1) what are we dragging, and does it have data?
        // if it does then we need to make a new slice
        // else we drag the current slice along the normal
        if (!camera_.dragged_slice()) {
            std::unique_ptr<slice> new_slice;
            int id = (*(camera_.get_slices().rbegin())).first + 1;
            int to_remove = -1;
            for (auto& id_the_slice : camera_.get_slices()) {
                auto& the_slice = id_the_slice.second;
                if (the_slice->hovered) {
                    if (the_slice->has_data()) {
                        new_slice = std::make_unique<slice>(id);
                        new_slice->orientation = the_slice->orientation;
                        to_remove = the_slice->id;
                        // FIXME need to generate a new id and upon 'popping'
                        // send a UpdateSlice packet
                        camera_.dragged_slice() = new_slice.get();
                    } else {
                        camera_.dragged_slice() = the_slice.get();
                    }
                    break;
                }
            }
            if (new_slice) {
                camera_.get_slices()[new_slice->id] = std::move(new_slice);
            }
            if (to_remove >= 0) {
                camera_.get_slices().erase(to_remove);
            }
            assert(camera_.dragged_slice());
        }

        auto slice = camera_.dragged_slice();
        auto& o = slice->orientation;

        auto axis1 = glm::vec3(o[0][0], o[0][1], o[0][2]);
        auto axis2 = glm::vec3(o[1][0], o[1][1], o[1][2]);
        auto normal = glm::normalize(glm::cross(axis1, axis2));

        // project the normal vector to screen coordinates
        // FIXME maybe need window matrix here too which would be kind of
        // painful maybe
        auto base_point_normal =
            glm::vec3(o[2][0], o[2][1], o[2][2]) + 0.5f * (axis1 + axis2);
        auto end_point_normal = base_point_normal + normal;

        auto a = camera_.matrix() * glm::vec4(base_point_normal, 1.0f);
        auto b = camera_.matrix() * glm::vec4(end_point_normal, 1.0f);
        auto normal_delta = b - a;
        float difference =
            glm::dot(glm::vec2(normal_delta.x, normal_delta.y), delta);

        // take the inner product of delta x and this normal vector

        auto dx = difference * normal;
        // FIXME check if it is still inside the bounding box of the volume
        // probably by checking all four corners are inside bounding box, should
        // define this box somewhere
        o[2][0] += dx[0];
        o[2][1] += dx[1];
        o[2][2] += dx[2];
    }

    drag_machine_kind kind() override { return drag_machine_kind::mover; }
};

}  // namespace tomovis
