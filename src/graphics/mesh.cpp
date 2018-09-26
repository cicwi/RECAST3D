#include <iostream>
#include <vector>

#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include "graphics/mesh.hpp"
#include "graphics/shader_program.hpp"

namespace tomovis {

Mesh::Mesh(aiMesh* asset_mesh) : asset_mesh_(asset_mesh) {
    if (!asset_mesh_->HasFaces()) {
        std::cout << "Only meshes with explicit faces supported!\n";
        return;
    }

    if (!asset_mesh_->HasNormals()) {
        std::cout << "Only meshes with normals supported!\n";
        return;
    }

    for (size_t i = 0; i < asset_mesh_->mNumFaces; ++i) {
        if (asset_mesh_->mFaces[i].mNumIndices != 3) {
            std::cout << "WARNING: Only triangulated meshes are supported! ("
                      << asset_mesh_->mFaces[i].mNumIndices << ")\n";
        }
    }

    std::cout << "Loaded mesh with: " << asset_mesh_->mNumVertices
              << " vertices\n";

    std::vector<unsigned int> indices;
    for (size_t i = 0; i < asset_mesh_->mNumFaces; ++i) {
        if (asset_mesh_->mFaces[i].mNumIndices != 3) {
            continue;
        }
        indices.push_back(asset_mesh_->mFaces[i].mIndices[0]);
        indices.push_back(asset_mesh_->mFaces[i].mIndices[1]);
        indices.push_back(asset_mesh_->mFaces[i].mIndices[2]);
    }
    index_count_ = indices.size();

    glGenVertexArrays(1, &vao_handle_);
    glBindVertexArray(vao_handle_);

    GLuint index_handle_;
    glGenBuffers(1, &index_handle_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_handle_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count_ * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, asset_mesh_->mNumVertices * 3 * sizeof(float),
                 (void*)asset_mesh_->mVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    std::cout << "Random vertex coordinate: "
              << asset_mesh_->mVertices[asset_mesh_->mNumVertices / 2][0] << "\n";

    glEnableVertexAttribArray(1);
    glGenBuffers(1, &vbo_normals_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals_handle_);
    glBufferData(GL_ARRAY_BUFFER, asset_mesh_->mNumVertices * 3 * sizeof(float),
                 (void*)asset_mesh_->mNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    mesh_rotate_ = glm::mat4(1.0);
    std::cout << "Rotate: " << glm::to_string(mesh_rotate_) << "\n";
}

Mesh::~Mesh() {
    // FIXME clean up buffers
}

void Mesh::animate(std::vector<PositionKeyframe> positions,
                   std::vector<RotationKeyframe> rotations, float speed,
                   float duration) {
    positions_ = positions;
    rotations_ = rotations;
    speed_ = speed;
    animated_ = true;
    animation_duration_ = duration;
}

void Mesh::transform(glm::mat4 transformation) {
    static_mesh_transformation_ = transformation;
}

template <typename Frame>
std::pair<Frame, Frame> find_two(std::vector<Frame> frames, float t) {
    // find frame
    auto current_frame =
        std::find_if(frames.begin(), frames.end(),
                     [&](auto frame) { return frame.time_step >= t; });

    if (current_frame == frames.begin()) {
        current_frame = frames.end();
    }
    --current_frame;

    auto f1 = *current_frame;

    current_frame++;
    if (current_frame == frames.end()) {
        current_frame = frames.begin();
    }

    auto f2 = *current_frame;

    return {f1, f2};
}

void Mesh::tick(float time_elapsed) {
    if (!animated_) {
        return;
    }

    auto cyclic_distance = [](float t1, float t2, float period) {
        auto result = t2 - t1;
        while (result > period) {
            result -= period;
        }
        while (result < 0) {
            result += period;
        }
        return result;
    };

    internal_time_ += speed_ * time_elapsed;
    auto cyclic_time = internal_time_;
    while (cyclic_time > animation_duration_) {
        cyclic_time -= animation_duration_;
    }

    auto frames = find_two(positions_, cyclic_time);
    auto f1 = frames.first;
    auto f2 = frames.second;
    if (f2.time_step == f1.time_step) {
        return;
    }
    auto alpha =
        cyclic_distance(f1.time_step, cyclic_time, animation_duration_) /
        cyclic_distance(f1.time_step, f2.time_step, animation_duration_);
    auto ipos = (1.0f - alpha) * f1.position + alpha * f2.position;

    auto rframes = find_two(rotations_, cyclic_time);
    auto g1 = rframes.first;
    auto g2 = rframes.second;
    if (g2.time_step == g1.time_step) {
        return;
    }
    auto beta =
        cyclic_distance(g1.time_step, cyclic_time, animation_duration_) /
        cyclic_distance(g1.time_step, g2.time_step, animation_duration_);

    assert(beta >= 0);
    assert(beta <= 1);

    mesh_translate_ = glm::translate(ipos);
    mesh_rotate_ =
        glm::mat4_cast(glm::slerp(g1.quaternion, g2.quaternion, beta));
}

void Mesh::draw(glm::mat4 world, glm::mat4 model, glm::vec3 camera_position,
                ShaderProgram* program) const {
    if (!visible_) {
        return;
    }

    if (!program) {
        return;
    }

    program->use();

    program->uniform("world_matrix", world);
    program->uniform("model_matrix", model);
    program->uniform("mesh_rotate", mesh_rotate_);
    program->uniform("mesh_translate",
                     static_mesh_transformation_);
    program->uniform("camera_position", camera_position);

    // set material
    program->uniform("material.ambient_color", material_.ambient_color);
    program->uniform("material.diffuse_color", material_.diffuse_color);
    program->uniform("material.specular_color", material_.specular_color);
    program->uniform("material.opacity", material_.opacity);
    program->uniform("material.shininess", material_.shininess);

    // draw with element buffer
    glBindVertexArray(vao_handle_);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);
}

} // namespace tomovis
