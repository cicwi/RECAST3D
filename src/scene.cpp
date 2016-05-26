#include "scene.hpp"
#include "graphics/scene_object_2d.hpp"


namespace tomovis {

Scene::Scene() {
   object_ = new SceneObject2D;     
}

Scene::~Scene() {
    delete object_;
}

void Scene::render() {
    object_->draw();
}

} // namespace tomovis
