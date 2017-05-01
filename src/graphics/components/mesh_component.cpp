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
    scene_ = aiImportFile("../data/clock_lowres.obj",
                              aiProcessPreset_TargetRealtime_Fast);

    if (scene_) {
        std::cout << "SCENE!\n";
        std::cout << "INFO:\n";
        std::cout << "> hasMeshes: " << scene_->HasMeshes() << "\n";
        std::cout << "> numMeshes: " << scene_->mNumMeshes << "\n";
        std::cout << "> numVertices0: " << scene_->mMeshes[0]->mNumVertices
                  << "\n";
        std::cout << "> numVertices1: " << scene_->mMeshes[1]->mNumVertices
                  << "\n";
        std::cout << "> numVertices2: " << scene_->mMeshes[2]->mNumVertices
                  << "\n";
    } else {
        std::cout << "Exit stage left!\n";
    }
}

MeshComponent::~MeshComponent() {
    if (scene_) {
        aiReleaseImport(scene_);
    }
}

void MeshComponent::describe() { ImGui::Checkbox("Show mesh", &show_); }

void MeshComponent::tick(float /* time_elapsed */) {}

void MeshComponent::draw(glm::mat4 /* world_to_screen */) {
    if (!show_) {
        return;
    }
}

} // namespace tomovis
