#include <iostream>
#include <queue>
#include <stack>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/model.hpp"
#include "graphics/node_animation.hpp"

#include <glm/gtc/type_ptr.hpp>

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

    auto vert =
#include "../src/shaders/basic_model.vert"
        ;
    auto frag =
#include "../src/shaders/basic_model.frag"
        ;

    program_ = std::make_unique<ShaderProgram>(vert, frag, false);
    //program_ = std::make_unique<ShaderProgram>("../src/shaders/basic_model.vert", "../src/shaders/basic_model.frag");
}

void Model::async_load_(std::string file) {
    progress_ = new ProgressUpdate;

    load_model_thread_ = std::thread([&, file]() {
        importer_ = new Assimp::Importer;
        importer_->SetProgressHandler(progress_);
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
    if (load_model_thread_.joinable()) {
        load_model_thread_.join();
    }

    if (importer_) {
        delete importer_;
    }
}

glm::mat4 Model::model_matrix() const {
    return glm::rotate(phi_, glm::vec3(0.0f, 1.0f, 0.0f)) *
           glm::scale(glm::vec3(scale_));
}

float Model::load_progress() { return progress_->progress(); }
void Model::cancel_load_() { progress_->keep_on_keeping_on_ = false; }

void Model::represent_() {

    // next, materials
    if (scene_->HasMaterials()) {
        std::cout << "Scene has " << scene_->mNumMaterials << " mats!\n";
    }

    std::vector<Material> materials;
    for (size_t i = 0; i < scene_->mNumMaterials; ++i) {
        Material material;
        auto mat = scene_->mMaterials[i];
        aiColor3D ambient;
        aiColor3D diffuse;
        aiColor3D specular;
        float opacity;
        float shininess;
        mat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        mat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        mat->Get(AI_MATKEY_OPACITY, opacity);
        mat->Get(AI_MATKEY_SHININESS, shininess);
        material.ambient_color = glm::vec3(ambient.r, ambient.g, ambient.b);
        material.diffuse_color = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
        material.specular_color = glm::vec3(specular.r, specular.g, specular.b);
        if (opacity <= 0.01f || opacity > 1.0f) {
            opacity = 1.0f;
        }
        std::cout << "Ambient: " << glm::to_string(material.ambient_color)
                  << "\n";
        std::cout << "Diffuse: " << glm::to_string(material.diffuse_color)
                  << "\n";
        std::cout << "Specular: " << glm::to_string(material.specular_color)
                  << "\n";
        std::cout << "Opacity: " << opacity << "\n";
        material.opacity = opacity;
        material.shininess = (int)shininess;
        materials.push_back(material);
    }

    for (size_t i = 0; i < scene_->mNumMeshes; ++i) {
        meshes_.push_back(std::make_unique<Mesh>(scene_->mMeshes[i]));
        meshes_[i]->mesh_material_ =
            materials[scene_->mMeshes[i]->mMaterialIndex];
        std::cout << "Mat idx: " << scene_->mMeshes[i]->mMaterialIndex << "\n";
        meshes_[i]->material() = meshes_[i]->mesh_material_;
    }

    // load animations
    // apply to proper meshes
    if (scene_->HasAnimations()) {
        std::cout << "Scene has animations!\n";

        std::cout << "Anim for node: "
                  << scene_->mAnimations[0]->mChannels[0]->mNodeName.C_Str()
                  << "\n";

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
                    frame.position =
                        glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);
                    positions.push_back(frame);
                }

                for (size_t k = 0; k < channel->mNumRotationKeys; ++k) {
                    RotationKeyframe frame;
                    auto key = channel->mRotationKeys[k];
                    frame.time_step = key.mTime;
                    frame.quaternion = glm::quat(key.mValue.w, key.mValue.x,
                                                 key.mValue.y, key.mValue.z);
                    rotations.push_back(frame);
                }

                auto node = scene_->mRootNode->FindNode(channel->mNodeName);
                for (size_t k = 0; k < node->mNumMeshes; ++k) {
                    meshes_[node->mMeshes[k]]->animate(positions, rotations,
                                                       speed, duration);
                }
            }
        }
    }

    // load transformations
    std::stack<std::pair<int, aiNode*>> nodes;
    nodes.push(std::make_pair(0, scene_->mRootNode));
    while (!nodes.empty()) {
        auto node_id = nodes.top();
        auto node = node_id.second;
        nodes.pop();

        std::cout << "| ";
        for (int i = 0; i < node_id.first; ++i) {
            std::cout << "> ";
        }
        std::cout << node->mName.C_Str() << "\n";

        for (size_t i = 0; i < node->mNumChildren; ++i) {
            nodes.push(std::make_pair(node_id.first + 1, node->mChildren[i]));
        }

        auto node_transform =
            glm::transpose(glm::make_mat4((float*)&node->mTransformation));
        for (size_t i = 0; i < node->mNumMeshes; ++i) {
            meshes_[node->mMeshes[i]]->transform(node_transform);
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

        phi_ += speed_ * (time_elapsed * twopi);
        while (phi_ > twopi) {
            phi_ -= twopi;
        }
    }

    for (auto& mesh : meshes_) {
        mesh->tick(time_elapsed);
    }
}

void Model::draw(glm::mat4 world_to_screen, glm::vec3 camera_position,
                 ShaderProgram* program) const {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    auto model = model_matrix();
    for (auto& mesh : meshes_) {
        mesh->draw(world_to_screen, model, camera_position,
                   program ? program : program_.get());
    }

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

}

} // namespace tomovis
