#include <iostream>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "graphics/mesh.hpp"
#include "graphics/model.hpp"

namespace tomovis {

Model::Model(std::string file) {
    std::cout << "Loading model: " << file << "...\n";

    scene_ = aiImportFile(file.c_str(), aiProcessPreset_TargetRealtime_Fast);

    if (!scene_) {
        // Exit ... stage left
        std::cout << "ERROR: Model not found: " << file << "\n";
        return;
    }

    for (size_t i = 0; i < scene_->mNumMeshes; ++i) {
        meshes_.push_back(std::make_unique<Mesh>(scene_->mMeshes[i]));
    }
}

Model::~Model() {
    if (scene_) {
        aiReleaseImport(scene_);
    }
}

void Model::draw(glm::mat4 world_to_screen) const {
    for (auto& mesh : meshes_) {
        mesh->draw(world_to_screen);
    }
}

} // namespace tomovis
