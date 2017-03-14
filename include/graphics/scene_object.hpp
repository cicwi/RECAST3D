#pragma once

#include <map>
#include <memory>
#include <vector>

#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include "components/object_component.hpp"

namespace tomovis {

class ShaderProgram;
class SceneCamera;

class SceneObject {
  public:
    SceneObject();
    virtual ~SceneObject();

    virtual void draw(glm::mat4 window_matrix) = 0;

    float& pixel_size() { return pixel_size_; }
    virtual SceneCamera& camera() { return *camera_; }

    virtual void set_data(std::vector<unsigned char>& data, int slice = 0) {}
    virtual void set_size(std::vector<int>& size, int slice = 0) {}

    void add_component(std::unique_ptr<ObjectComponent> component) {
        components_.insert(std::make_pair(component->identifier(), std::move(component)));
    }

    ObjectComponent& get_component(std::string identifier) {
        return *components_[identifier].get();
    }

  protected:
    virtual void update_image_(int slice = 0) { }

    GLuint vao_handle_;
    GLuint vbo_handle_;
    std::unique_ptr<ShaderProgram> program_;
    std::unique_ptr<SceneCamera> camera_;
    float pixel_size_ = 1.0;

    // FIXME: map of components
    std::map<std::string, std::unique_ptr<ObjectComponent>> components_;
};

} // namespace tomovis
