#include <glm/gtc/type_ptr.hpp>

#include "math_common.hpp"

namespace tomovis {

glm::mat4 create_orientation_matrix(glm::vec3 base, glm::vec3 x, glm::vec3 y) {
    float orientation_matrix[16] = {x.x,  y.x,  base.x, 0.0f,  // 1
                                    x.y,  y.y,  base.y, 0.0f,  // 2
                                    x.z,  y.z,  base.z, 0.0f,  // 3
                                    0.0f, 0.0f, 0.0f,   1.0f}; // 4

    return glm::transpose(glm::make_mat4(orientation_matrix));
}

} // namespace tomovis
