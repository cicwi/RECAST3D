#include <iostream>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "graphics/components/mesh_component.hpp"
#include "graphics/scene_camera_3d.hpp"

namespace tomovis {

MeshComponent::MeshComponent(SceneObject& object, int scene_id)
    : object_(object), scene_id_(scene_id) {
    auto scene = aiImportFile("../data/clock_lowres.obj",
                              aiProcessPreset_TargetRealtime_Fast);

    if (scene) {
        std::cout << "SCENE!\n";
        //get_bounding_box(&scene_min, &scene_max);
        //scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
        //scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
        //scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
    }
    std::cout << "Exit stage left!\n";
}

MeshComponent::~MeshComponent() {}

void MeshComponent::describe() { ImGui::Checkbox("Show mesh", &show_); }

void MeshComponent::tick(float time_elapsed) {}

void MeshComponent::draw(glm::mat4 world_to_screen) const {
    if (!show_) {
        return;
    }
}

} // namespace tomovis
