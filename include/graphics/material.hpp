#pragma once

#include <glm/glm.hpp>

namespace tomovis {

struct Material {
    glm::vec3 ambient_color = glm::vec3(0.1f);
    glm::vec3 diffuse_color = glm::vec3(0.4f);
    glm::vec3 specular_color = glm::vec3(0.8f);
    float opacity = 1.0f;
    float shininess = 8.0f;
};

} // namespace tomovis
