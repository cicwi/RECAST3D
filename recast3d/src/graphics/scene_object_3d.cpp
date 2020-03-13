#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/scene_camera_3d.hpp"
#include "graphics/scene_object_3d.hpp"
#include "graphics/shader_program.hpp"

#include "graphics/components/axes_component.hpp"
#include "graphics/components/control_component.hpp"

namespace tomovis {

SceneObject3d::SceneObject3d(int scene_id) : SceneObject(scene_id) {
  camera_ = std::make_unique<SceneCamera3d>();
  this->add_component(std::make_unique<AxesComponent>(*this, scene_id));
  this->add_component(std::make_unique<ControlComponent>(*this, scene_id));
}

SceneObject3d::~SceneObject3d() {}

void SceneObject3d::draw(glm::mat4 window_matrix) {
  auto world_to_screen = window_matrix * camera_->matrix();
  std::vector<ObjectComponent *> drawables;
  for (auto &component : components_) {
    drawables.push_back(component.second.get());
  }

  std::sort(drawables.begin(), drawables.end(), [](auto lptr, auto rptr) {
    return lptr->priority() > rptr->priority();
  });

  for (auto drawable : drawables) {
    drawable->draw(world_to_screen);
  }
}

} // namespace tomovis
