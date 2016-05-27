#pragma once

#include <vector>
#include <utility>
#include <string>

#include <glm/glm.hpp>

#include "configurable.hpp"
#include "input_handler.hpp"

namespace tomovis {

class SceneCamera : public InputHandler {
  public:
    SceneCamera(){};
    virtual ~SceneCamera(){};

    virtual glm::mat4 matrix() = 0;
    virtual std::vector<parameter<float>>& parameters() = 0;

};

} // namespace tomovis
