#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/components/reconstruction_component.hpp"
#include "graphics/components/geometry_component.hpp"
#include "graphics/components/partitioning_component.hpp"
#include "graphics/scene_camera_3d.hpp"
#include "graphics/scene_object_3d.hpp"
#include "graphics/shader_program.hpp"

namespace tomovis {

SceneObject3d::SceneObject3d(int scene_id) : SceneObject(scene_id) {
    camera_ = std::make_unique<SceneCamera3d>();

    this->add_component(std::make_unique<ReconstructionComponent>(*this, this->scene_id_));
    this->add_component(std::make_unique<GeometryComponent>(*this, this->scene_id_));
    this->add_component(std::make_unique<PartitioningComponent>(*this, this->scene_id_));
}

SceneObject3d::~SceneObject3d() {}

void SceneObject3d::draw(glm::mat4 window_matrix) {
    auto world_to_screen = window_matrix * camera_->matrix();
    for (auto& component : components_) {
        component.second->draw(world_to_screen);
    }
}

}  // namespace tomovis
