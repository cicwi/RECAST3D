#pragma once

#include "scene_object.hpp"


namespace tomovis {

class SceneObject2d : public SceneObject {
  public:
    SceneObject2d(int scene_id);
    ~SceneObject2d();

    void draw(glm::mat4 window_matrix) override;

    void set_data(std::vector<unsigned char>& data, int slice = 0) override {
        if (slice != 0)
            throw;

        data_ = data;
        update_image_();
    }

    void set_size(std::vector<int>& size, int slice = 0) {
        (void)slice;
        size_ = size;
    }

  protected:
    void update_image_(int slice = 0) override;

  private:
    GLuint texture_id_;
    std::vector<unsigned char> data_;
    std::vector<int> size_;
};

} // namespace tomovis
