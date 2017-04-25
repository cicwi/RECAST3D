#include <iostream>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>

#include "graphics/components/movie_component.hpp"
#include "graphics/scene_camera_3d.hpp"

namespace tomovis {

MovieComponent::MovieComponent(SceneObject& object, int scene_id)
    : object_(object), scene_id_(scene_id), model_("../data/clock_lowres.obj") {
}

MovieComponent::~MovieComponent() {
}

void MovieComponent::describe() {}

void MovieComponent::tick(float /* time_elapsed */) {}

void MovieComponent::draw(glm::mat4 world_to_screen) const {
    model_.draw(world_to_screen);
}

} // namespace tomovis
