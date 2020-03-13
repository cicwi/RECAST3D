#include <memory>
#include <iostream>

#include <tomop/tomop.hpp>

#include "graphics/scene_object_2d.hpp"
#include "graphics/scene_object_3d.hpp"
#include "scene.hpp"

namespace tomovis {

Scene::Scene(std::string name, int dimension, int scene_id)
    : name_(name), dimension_(dimension), scene_id_(scene_id) {
    if (dimension_ == 2) {
        object_ = std::make_unique<SceneObject2d>(scene_id_);
    } else if (dimension_ == 3) {
        object_ = std::make_unique<SceneObject3d>(scene_id_);
    } else {
        throw;
    }
}

Scene::~Scene() {
    auto packet = KillScenePacket(scene_id_);
    object_->send(packet);
}

void Scene::render(glm::mat4 window_matrix) { object_->draw(window_matrix); }

} // namespace tomovis
