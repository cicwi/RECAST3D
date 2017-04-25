#pragma once

#include <string>

#include "render_target.hpp"

struct aiScene;

namespace tomovis {

class Model {
  public:
    Model(std::string file);
    ~Model();

    void draw(glm::mat4 world_to_screen) const;

  private:
    const aiScene* scene_;
};

} // namespace tomovis
