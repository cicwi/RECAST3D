#pragma once

#include <vector>
#include <utility>
#include <string>

#include <glm/glm.hpp>

#include "configurable.hpp"

namespace tomovis {

class SceneCamera {
  public:
    SceneCamera(){};
    virtual ~SceneCamera(){};

    virtual glm::mat4 matrix() = 0;

    virtual std::vector<parameter<float>>& parameters() = 0;
};

} // namespace tomovis
