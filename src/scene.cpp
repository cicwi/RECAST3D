#include "scene.hpp"
#include "graphics/scene_object_2d.hpp"


namespace tomovis {

Scene::Scene() {
   object_ = new SceneObject2d;     
}

Scene::~Scene() {
    delete object_;
}

void Scene::render(glm::mat4 window_matrix) {
    object_->draw(window_matrix);
}

} // namespace tomovis
