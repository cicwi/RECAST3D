#include "scene.hpp"
#include "graphics/scene_object.hpp"


namespace tomovis {

Scene::Scene() {
   object_ = new SceneObject;     
}

Scene::~Scene() {
    delete object_;
}

void Scene::render() {
    object_->draw();
}

} // namespace tomovis
