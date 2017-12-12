#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "graphics/render_target.hpp"
#include "graphics/scene_object.hpp"

namespace tomovis {

class Scene : public RenderTarget {
   public:
    Scene(std::string name, int dimension, int scene_id);
    ~Scene();

    void render(glm::mat4 window_matrix) override;

    SceneObject& object() { return *object_.get(); }

    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = name; }

    void set_data(std::vector<unsigned char>& data, int slice = 0) {
        object_->set_data(data, slice);
    }

    void set_size(std::vector<int>& size, int slice = 0) { object_->set_size(size, slice); }

    int dimension() { return dimension_; }

   private:
    std::unique_ptr<SceneObject> object_;
    std::string name_;
    int dimension_;
    int scene_id_;
};

}  // namespace tomovis
