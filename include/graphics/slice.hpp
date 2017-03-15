#pragma once

#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include "textures.hpp"

namespace tomovis {

struct slice {
    slice(int id_);

    void update_texture();
    void set_orientation(glm::vec3 base, glm::vec3 x, glm::vec3 y);

    std::vector<unsigned char> data;
    std::vector<int> size;

    int id = -1;
    bool hovered = false;
    bool inactive = false;
    bool has_data() { return !data.empty(); }
    bool transparent() { return hovered || !has_data(); }

    auto& get_texture() { return tex_; }

    texture<unsigned char> tex_;
    glm::mat4 orientation;
};

}  // namespace tomovis
