#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "graphics/mesh.hpp"
#include "graphics/model.hpp"

namespace tomovis {

class ProgressUpdate : public Assimp::ProgressHandler {
  public:
    bool Update(float percentage) override {
        percentage_ = percentage;
        return keep_on_keeping_on_;
    }

    float progress() { return percentage_; }

  private:
    friend Model;

    float percentage_ = -1.0f;
    bool keep_on_keeping_on_ = true;
};

Model::Model(std::string file) {
    std::cout << "Loading model: '" << file << "' ...\n";
    async_load_(file);
}

void Model::async_load_(std::string file) {
    progress_ = std::make_unique<ProgressUpdate>();

    load_model_thread_ = std::thread([&, file]() {
        importer_ = std::make_unique<Assimp::Importer>();
        importer_->SetProgressHandler(progress_.get());
        scene_ = importer_->ReadFile(file, aiProcessPreset_TargetRealtime_Fast);

        if (!scene_) {
            // Exit ... stage left
            std::cout << "ERROR: Model not loaded: " << file << "\n";
            return;
        }

        to_load_ = true;
    });
}

Model::~Model() {
    if (scene_) {
        aiReleaseImport(scene_);
    }
    load_model_thread_.join();
}

glm::mat4 Model::model_matrix() const {
    return glm::rotate(phi_, glm::vec3(0.0f, 1.0f, 0.0f));
}

float Model::load_progress() { return progress_->progress(); }
void Model::cancel_load_() { progress_->keep_on_keeping_on_ = false; }

void Model::tick(float time_elapsed) {
    if (scene_ && to_load_) {
        for (size_t i = 0; i < scene_->mNumMeshes; ++i) {
            meshes_.push_back(std::make_unique<Mesh>(scene_->mMeshes[i]));
        }
        to_load_ = false;
    }

    const float twopi = 2.0f * glm::pi<float>();

    phi_ += time_elapsed;
    while (phi_ > twopi) {
        phi_ -= twopi;
    }
}

void Model::draw(glm::mat4 world_to_screen, glm::vec3 camera_position) const {
    auto model = model_matrix();
    for (auto& mesh : meshes_) {
        mesh->draw(world_to_screen, model, camera_position);
    }
}

} // namespace tomovis
