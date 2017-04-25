#include <iostream>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "graphics/model.hpp"

namespace tomovis {

Model::Model(std::string file) {
    scene_ = aiImportFile(file.c_str(),
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
        // Exit ... stage left
        std::cout << "ERROR: Model not found: " << file << "\n";
    }
}

Model::~Model() {
    if (scene_) {
        aiReleaseImport(scene_);
    }
}

void Model::draw(glm::mat4 world_to_screen) const {
    (void)world_to_screen;
}

} // namespace tomovis
