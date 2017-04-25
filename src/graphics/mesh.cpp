#include <iostream>
#include <vector>

#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "graphics/mesh.hpp"

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
            std::cout << "Only meshes with triangles as supported!\n";
            return;
        }
    }

    std::vector<unsigned int> indices;
    for (size_t i = 0; i < asset_mesh_->mNumFaces; ++i) {
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

    glEnableVertexAttribArray(1);
    glGenBuffers(1, &vbo_normals_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals_handle_);
    glBufferData(GL_ARRAY_BUFFER, asset_mesh_->mNumVertices * 3 * sizeof(float),
                 (void*)asset_mesh_->mNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    program_ = std::make_unique<ShaderProgram>(
        "../src/shaders/basic_model.vert", "../src/shaders/basic_model.frag");
}

Mesh::~Mesh() {}

void Mesh::draw(glm::mat4 model_to_screen) const {
    glEnable(GL_DEPTH_TEST);

    model_to_screen = model_to_screen * glm::scale(glm::vec3(1.0f));

    program_->use();
    GLint transform_loc =
        glGetUniformLocation(program_->handle(), "transform_matrix");
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, &model_to_screen[0][0]);

    // draw with element buffer
    glBindVertexArray(vao_handle_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_handle_);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, (void*)0);

    glDisable(GL_DEPTH_TEST);
}

} // namespace tomovis
