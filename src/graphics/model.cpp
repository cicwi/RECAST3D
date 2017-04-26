#include <iostream>
#include <queue>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "graphics/mesh.hpp"
#include "graphics/model.hpp"
#include "graphics/node_animation.hpp"

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

void Model::represent_() {
    for (size_t i = 0; i < scene_->mNumMeshes; ++i) {
        meshes_.push_back(std::make_unique<Mesh>(scene_->mMeshes[i]));
    }

    // load animations
    // apply to proper meshes
    if (scene_->HasAnimations()) {
        std::cout << "Scene has animations!\n";
        std::cout << "#: " << scene_->mNumAnimations << "\n";
        std::cout << "#c1: " << scene_->mAnimations[0]->mNumMeshChannels << "\n";
        std::cout << "#c2: " << scene_->mAnimations[0]->mNumChannels << "\n";

        std::queue<aiNode*> nodes;
        nodes.push(scene_->mRootNode);
        while (!nodes.empty()) {
            auto node = nodes.front();
            nodes.pop();
            for (size_t i = 0; i < node->mNumChildren; ++i) {
                nodes.push(node->mChildren[i]);
            }

            std::cout << "Node: " << node->mName.C_Str() << "\n";
        }

        std::cout << "Anim for node: " << scene_->mAnimations[0]->mChannels[0]->mNodeName.C_Str() << "\n";

        for (size_t i = 0; i < scene_->mNumAnimations; ++i) {
            auto anim = scene_->mAnimations[i];
            float speed = anim->mTicksPerSecond;
            float duration = anim->mDuration;
            for (size_t j = 0; j < anim->mNumChannels; ++j) {
                auto channel = anim->mChannels[j];
                // gather frames for channel
                std::vector<PositionKeyframe> positions;
                std::vector<RotationKeyframe> rotations;

                for (size_t k = 0; k < channel->mNumPositionKeys; ++k) {
                    PositionKeyframe frame;
                    auto key = channel->mPositionKeys[k];
                    frame.time_step = key.mTime;
                    frame.position = glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);
                    positions.push_back(frame);
                }

                for (size_t k = 0; k < channel->mNumRotationKeys; ++k) {
                    RotationKeyframe frame;
                    auto key = channel->mRotationKeys[k];
                    frame.time_step = key.mTime;
                    frame.quaternion = glm::vec4(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);
                    rotations.push_back(frame);
                }

                auto node = scene_->mRootNode->FindNode(channel->mNodeName);
                for (size_t k = 0; k < node->mNumMeshes; ++k) {
                    meshes_[node->mMeshes[k]]->animate(positions, rotations, speed, duration);
                }
            }
        }
    }

    to_load_ = false;
}

void Model::tick(float time_elapsed) {
    if (scene_ && to_load_) {
        represent_();
    }

    if (paused_) {
        return;
    }

    if (rotate_) {
        const float twopi = 2.0f * glm::pi<float>();

        phi_ += 0.1f * time_elapsed;
        while (phi_ > twopi) {
            phi_ -= twopi;
        }
    }

    for (auto& mesh : meshes_) {
        mesh->tick(time_elapsed);
    }
}

void Model::draw(glm::mat4 world_to_screen, glm::vec3 camera_position) const {
    auto model = model_matrix();
    for (auto& mesh : meshes_) {
        mesh->draw(world_to_screen, model, camera_position);
    }
}

} // namespace tomovis
