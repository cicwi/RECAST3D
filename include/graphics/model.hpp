#pragma once

#include <string>
#include <thread>
#include <vector>

#include "ticker.hpp"

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

    void draw(glm::mat4 world_to_screen, glm::vec3 camera_position) const;
    void tick(float time_elapsed) override;

    glm::mat4 model_matrix() const;

    float load_progress();

    void toggle_pause() { paused_ = !paused_; }
    void toggle_rotate() { rotate_ = !rotate_; }

    float& scale() { return scale_; }

  private:
    const aiScene* scene_;
    std::vector<std::unique_ptr<Mesh>> meshes_;

    void represent_();

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
};

} // namespace tomovis
