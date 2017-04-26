#pragma once

#include <glm/glm.hpp>

namespace tomovis {

struct Material {
    glm::vec3 ambient_color = glm::vec3(0.8f);
    glm::vec3 diffuse_color = glm::vec3(0.8f);
    glm::vec3 specular_color = glm::vec3(0.8f);
    float opacity = 1.0f;
    int shininess = 32;
};

} // namespace tomovis
