#include <memory>

#include "graphics/scene_object_2d.hpp"
#include "graphics/scene_object_3d.hpp"
#include "scene.hpp"

namespace tomovis {

Scene::Scene(std::string name, int dimension)
    : name_(name), dimension_(dimension) {
    if (dimension_ == 2) {
        object_ = std::make_unique<SceneObject2d>();
    } else if (dimension_ == 3) {
        object_ = std::make_unique<SceneObject3d>();
    } else {
        throw;
    }
}

void Scene::render(glm::mat4 window_matrix) { object_->draw(window_matrix); }

}  // namespace tomovis
