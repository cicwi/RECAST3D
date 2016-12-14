#include <memory>

#include "scene.hpp"
#include "graphics/scene_object_2d.hpp"

namespace tomovis {

Scene::Scene(std::string name) : name_(name) {
   object_ = std::make_unique<SceneObject2d>();
}

void Scene::render(glm::mat4 window_matrix) {
    object_->draw(window_matrix);
}

} // namespace tomovis
