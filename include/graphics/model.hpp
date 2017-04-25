#pragma once

#include <string>
#include <memory>
#include <vector>

#include "render_target.hpp"

struct aiScene;

namespace tomovis {

class Mesh;

class Model {
  public:
    Model(std::string file);
    ~Model();

    void draw(glm::mat4 world_to_screen) const;

  private:
    const aiScene* scene_;
    std::vector<std::unique_ptr<Mesh>> meshes_;
};

} // namespace tomovis
