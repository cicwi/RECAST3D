#pragma once

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "graphics/shader_program.hpp"
#include "graphics/slice.hpp"
#include "graphics/textures.hpp"
#include "object_component.hpp"

namespace tomovis {

class ReconstructionComponent : public ObjectComponent {
   public:
    ReconstructionComponent();
    ~ReconstructionComponent();

    void draw(glm::mat4 world_to_screen) const override;
    std::string identifier() const override { return "reconstruction"; }

    void set_size(std::vector<int>& size, int slice = 0) {
        if (slice < 0 || slice >= (int)slices_.size()) throw;
        slices_[slice]->size = size;
    }

    void set_data(std::vector<unsigned char>& data, int slice = 0) {
        if (slice < 0 || slice >= (int)slices_.size()) throw;

        slices_[slice]->data = data;
        update_image_(slice);
    }

    void set_volume_data(std::vector<int>& volume_size,
                         std::vector<unsigned char>& data) {
        assert(volume_size.size() == 3);
        volume_texture_.set_data(volume_size[0], volume_size[1], volume_size[2],
                                 data);
    }

    std::map<int, std::unique_ptr<slice>>& slices() { return slices_; }

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
};

}  // namespace tomovis
