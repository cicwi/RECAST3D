#pragma once

#include <array>
#include <cstddef>

#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include "textures.hpp"

namespace tomovis {

struct slice {
    slice(int id_);

    void update_texture();
    void set_orientation(glm::vec3 base, glm::vec3 x, glm::vec3 y);

    std::vector<uint32_t> data;

    int id = -1;
    int replaces_id = -1;
    bool hovered = false;
    bool inactive = false;
    bool has_data() { return !data.empty(); }
    bool transparent() { return hovered || !has_data(); }

    std::vector<int> size;

    auto& get_texture() { return tex_; }

    texture<uint32_t> tex_;
    glm::mat4 orientation;

    std::array<float, 9> packed_orientation() {
        return std::array<float, 9>{orientation[0][0], orientation[0][1], orientation[0][2],
                orientation[1][0], orientation[1][1], orientation[1][2],
                orientation[2][0], orientation[2][1], orientation[2][2]};
    }
};

}  // namespace tomovis
