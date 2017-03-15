#include <glm/gtc/type_ptr.hpp>

#include "graphics/slice.hpp"

namespace tomovis {

slice::slice(int id_) : id(id_), size{32, 32}, tex_(32, 32) {
    update_texture();
}


void slice::update_texture() {
    tex_.set_data(data, size[0], size[1]);
}

void slice::set_orientation(glm::vec3 base, glm::vec3 x, glm::vec3 y) {
    float orientation_matrix[16] = {x.x,  y.x,  base.x, 0.0f,   // 1
                                    x.y,  y.y,  base.y, 0.0f,   // 2
                                    x.z,  y.z,  base.z, 0.0f,   // 3
                                    0.0f, 0.0f, 0.0f,   1.0f};  // 4

    orientation = glm::transpose(glm::make_mat4(orientation_matrix));
}

}  // namespace tomovis
