#pragma once

#include <glm/glm.hpp>

namespace tomovis {

struct Keyframe {
    float time_step;
};

struct PositionKeyframe : public Keyframe {
    glm::vec3 position;
};

struct RotationKeyframe : public Keyframe {
    glm::vec4 quaternion;
};

} // namespace tomovis
