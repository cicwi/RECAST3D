#pragma once

#include <memory>
#include <iostream>
#include <vector>

#include "graphics/render_target.hpp"
#include "graphics/scene_object.hpp"

namespace tomovis {

class Scene : public RenderTarget {
   public:
    Scene(std::string name);

    void render(glm::mat4 window_matrix) override;

    SceneObject& object() { return *object_.get(); }

    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = name; }

    void set_data(std::vector<unsigned char>& data) {
        object_->set_data(data);
    }

    void set_size(std::vector<int>& size) {
        object_->set_size(size);
    }

   private:
    std::unique_ptr<SceneObject> object_;
    std::string name_;
};

}  // namespace tomovis
