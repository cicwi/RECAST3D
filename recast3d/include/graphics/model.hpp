#pragma once

#include <string>
#include <thread>
#include <vector>

#include "ticker.hpp"
#include "shader_program.hpp"

struct aiScene;

namespace Assimp {
class Importer;
}

namespace tomovis {

class Mesh;
class ProgressUpdate;

class Model : public Ticker {
  public:
    Model(std::string file);
    ~Model();

    void draw(glm::mat4 world_to_screen, glm::vec3 camera_position,
                 ShaderProgram* program = nullptr) const;
    void tick(float time_elapsed) override;

    glm::mat4 model_matrix() const;

    float load_progress();

    void toggle_pause() { paused_ = !paused_; }
    void toggle_rotate() { rotate_ = !rotate_; }

    void rotate(bool should_rotate) { rotate_ = should_rotate; }
    void rotations_per_second(float rps) { speed_ = rps; }

    float& scale() { return scale_; }
    float& phi() { return phi_; }

    std::vector<std::unique_ptr<Mesh>>& meshes() { return meshes_; }

  private:
    const aiScene* scene_;
    std::vector<std::unique_ptr<Mesh>> meshes_;

    void represent_();

    float speed_ = 0.25f;
    float scale_ = 1.0f;
    float phi_ = 0.0f;
    void cancel_load_();

    std::thread load_model_thread_;
    void async_load_(std::string file);
    ProgressUpdate* progress_;

    Assimp::Importer* importer_;

    bool to_load_ = false;
    bool paused_ = false;
    bool rotate_ = false;

    // FIXME: replace with single program
    std::unique_ptr<ShaderProgram> program_;
};

} // namespace tomovis
