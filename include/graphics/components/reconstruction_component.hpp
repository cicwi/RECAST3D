#pragma once

#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "graphics/scene_object.hpp"
#include "graphics/shader_program.hpp"
#include "graphics/slice.hpp"
#include "graphics/textures.hpp"
#include "object_component.hpp"
#include "util.hpp"

namespace tomovis {

class ReconstructionComponent;

enum class recon_drag_machine_kind : int {
    none,
    rotator,
    translator,
};

class ReconDragMachine {
  public:
    ReconDragMachine(ReconstructionComponent& comp, glm::vec2 initial)
        : comp_(comp), initial_(initial) {}

    virtual void on_drag(glm::vec2 delta) = 0;
    virtual recon_drag_machine_kind kind() = 0;

  protected:
    ReconstructionComponent& comp_;
    glm::vec2 initial_;
};

class ReconstructionComponent : public ObjectComponent {
  public:
    ReconstructionComponent(SceneObject& object, int scene_id);
    ~ReconstructionComponent();

    void draw(glm::mat4 world_to_screen) override;
    void describe() override;

    void set_data(std::vector<float>& data, std::array<int32_t, 2> size,
                  int slice, bool additive = true);
    void update_partial_slice(std::vector<float>& data,
                              std::array<int32_t, 2> offset,
                              std::array<int32_t, 2> size,
                              std::array<int32_t, 2> global_size, int slice,
                              bool additive = true);
    void set_volume_data(std::vector<float>& data,
                         std::array<int32_t, 3>& volume_size);
    void update_partial_volume(std::vector<float>& data,
                               std::array<int32_t, 3>& offset,
                               std::array<int32_t, 3>& size,
                               std::array<int32_t, 3>& global_size);
    void set_volume_position(glm::vec3 min_pt, glm::vec3 max_pt);
    void update_histogram(const std::vector<float>& data);

    void send_slices();

    void switch_if_necessary(recon_drag_machine_kind kind);
    bool handle_mouse_button(int button, bool down) override;
    bool handle_mouse_moved(float x, float y) override;
    int index_hovering_over(float x, float y);
    void check_hovered(float x, float y);

    std::map<int, std::unique_ptr<slice>>& slices() { return slices_; }
    glm::mat4 volume_transform() { return volume_transform_; }
    auto scene_id() { return scene_id_; }
    auto& object() { return object_; }
    auto& dragged_slice() { return dragged_slice_; }
    auto hovered_slice() { return hovered_slice_; }
    auto& get_slices() { return slices_; }
    std::string identifier() const override { return "reconstruction"; }

    std::tuple<bool, float, glm::vec3> intersection_point(glm::mat4 inv_matrix,
                                                          glm::mat4 orientation,
                                                          glm::vec2 point);

    std::pair<float, float> overall_min_and_max();

    auto generate_slice_idx() { return next_idx_++; }

  private:
    void update_image_(slice* s);

    std::map<int, std::unique_ptr<slice>> slices_;
    std::map<int, std::unique_ptr<slice>> fixed_slices_;

    glm::mat4 volume_transform_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;

    GLuint line_vao_handle_;
    GLuint line_vbo_handle_;

    GLuint cube_vao_handle_;
    GLuint cube_vbo_handle_;
    GLuint cube_index_handle_;
    int cube_index_count_;
    std::unique_ptr<ShaderProgram> cube_program_;

    SceneObject& object_;
    int next_idx_ = 3;

    GLuint colormap_texture_;
    texture3d<float> volume_texture_;
    std::vector<float> volume_data_;

    std::unique_ptr<ReconDragMachine> drag_machine_;
    slice* dragged_slice_ = nullptr;
    slice* hovered_slice_ = nullptr;

    std::vector<float> histogram_;

    float prev_x_ = -1.1f;
    float prev_y_ = -1.1f;

    float lower_value_ = -1.0f;
    float upper_value_ = 1.0f;
  bool value_not_set_ = true;
    float volume_min_ = 0.0f;
    float volume_max_ = 1.0f;

    bool dragging_ = false;
    bool hovering_ = false;
    bool show_ = true;
    bool transparency_mode_ = false;

    int scene_id_;
};

class SliceTranslator : public ReconDragMachine {
  public:
    using ReconDragMachine::ReconDragMachine;

    void on_drag(glm::vec2 delta) override;
    recon_drag_machine_kind kind() override {
        return recon_drag_machine_kind::translator;
    }
};

class SliceRotator : public ReconDragMachine {
  public:
    SliceRotator(ReconstructionComponent& comp, glm::vec2 initial);

    void on_drag(glm::vec2 delta) override;

    recon_drag_machine_kind kind() override {
        return recon_drag_machine_kind::rotator;
    }

    glm::vec3 rot_base;
    glm::vec3 rot_end;
    glm::vec2 screen_direction;
};

} // namespace tomovis
