#include <glm/gtc/type_ptr.hpp>

#include "graphics/slice.hpp"
#include "math_common.hpp"
#include "util.hpp"

namespace tomovis {

slice::slice(int id_) : id(id_), size{32, 32}, tex_(32, 32) {
    update_texture();
}

void slice::update_texture() {
    if (!has_data()) {
        return;
    }
    tex_.set_data(data, size[0], size[1]);
}

void slice::set_orientation(glm::vec3 base, glm::vec3 x, glm::vec3 y) {
    orientation = create_orientation_matrix(base, x, y);
}

} // namespace tomovis
