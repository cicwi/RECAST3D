#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "graphics/shader_program.hpp"
#include "graphics/slice.hpp"
#include "graphics/textures.hpp"
#include "graphics/scene_object.hpp"
#include "object_component.hpp"

namespace tomovis {

class ReconstructionComponent;

enum class recon_drag_machine_kind : int {
    none,
    rotator,
    translator,
};

class ReconDragMachine {
   public:
    ReconDragMachine(ReconstructionComponent& comp) : comp_(comp) {}

    virtual void on_drag(glm::vec2 delta) = 0;
    virtual recon_drag_machine_kind kind() = 0;

   protected:
    ReconstructionComponent& comp_;
};

class ReconstructionComponent : public ObjectComponent {
   public:
    ReconstructionComponent(SceneObject& object, int scene_id);
    ~ReconstructionComponent();

    void draw(glm::mat4 world_to_screen) const override;
    std::string identifier() const override { return "reconstruction"; }

    void set_size(std::vector<int>& size, int slice = 0) {
        if (slices_.find(slice) == slices_.end()) {
            std::cout << "Updating inactive slice: " << slice << "\n";
            return;
        }
        slices_[slice]->size = size;
    }

    void set_data(std::vector<unsigned char>& data, int slice = 0) {
        if (slices_.find(slice) == slices_.end()) {
            std::cout << "Updating inactive slice: " << slice << "\n";
            return;
        }
        slices_[slice]->data = data;
        update_image_(slice);
    }

    void set_volume_data(std::vector<int>& volume_size,
                         std::vector<unsigned char>& data) {
        assert(volume_size.size() == 3);
        volume_texture_.set_data(volume_size[0], volume_size[1], volume_size[2],
                                 data);
    }

    bool handle_mouse_button(int button, bool down) override;
    bool handle_mouse_moved(float x, float y) override;

    int index_hovering_over(float x, float y);
    void check_hovered(float x, float y);
    void switch_if_necessary(recon_drag_machine_kind kind);

    std::map<int, std::unique_ptr<slice>>& slices() { return slices_; }

    auto& object() { return object_; }
    auto& dragged_slice() { return dragged_slice_; }
    auto& get_slices() { return slices_; }

   private:
    void update_image_(int slice);

    std::map<int, std::unique_ptr<slice>> slices_;

    glm::vec3 box_origin_;
    glm::vec3 box_size_;

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;

    GLuint cube_vao_handle_;
    GLuint cube_vbo_handle_;
    std::unique_ptr<ShaderProgram> cube_program_;

    GLuint colormap_texture_;
    texture3d<unsigned char> volume_texture_;

    std::unique_ptr<ReconDragMachine> drag_machine_;
    slice* dragged_slice_ = nullptr;
    SceneObject& object_;

    float prev_x_ = -1.1f;
    float prev_y_ = -1.1f;

    bool dragging_ = false;
    bool hovering_ = false;

    int scene_id_;
};

class SliceTranslator : public ReconDragMachine {
   public:
    using ReconDragMachine::ReconDragMachine;

    void on_drag(glm::vec2 delta) override;
    recon_drag_machine_kind kind() override { return recon_drag_machine_kind::translator; }
};

}  // namespace tomovis
