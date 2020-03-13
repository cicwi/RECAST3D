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

    std::vector<float> data;

    void add_data(std::vector<float> other) {
        for (auto i = 0u; i < data.size(); ++i) {
            data[i] += other[i];
        }
    }

    void add_partial_data(std::vector<float> other,
                          std::array<int32_t, 2> offset,
                          std::array<int32_t, 2> partial_size) {
        int idx = 0;
        for (auto j = offset[1]; j < partial_size[1] + offset[1]; ++j) {
            for (auto i = offset[0]; i < partial_size[0] + offset[0]; ++i) {
                data[j * size[0] + i] += other[idx];
            }
        }
    }

    int id = -1;
    int replaces_id = -1;
    bool hovered = false;
    bool inactive = false;
    bool has_data() { return !data.empty(); }
    bool transparent() { return hovered || !has_data(); }

    std::array<int32_t, 2> size;

    auto& get_texture() { return tex_; }

    texture<float> tex_;
    glm::mat4 orientation;

    std::array<float, 9> packed_orientation() {
        return std::array<float, 9>{
            orientation[0][0], orientation[0][1], orientation[0][2],
            orientation[1][0], orientation[1][1], orientation[1][2],
            orientation[2][0], orientation[2][1], orientation[2][2]};
    }

    float min_value = 1.0f;
    float max_value = -1.0f;
};

} // namespace tomovis
