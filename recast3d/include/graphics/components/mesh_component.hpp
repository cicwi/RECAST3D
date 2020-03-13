#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/scene_object.hpp"
#include "graphics/shader_program.hpp"
#include "object_component.hpp"

struct aiScene;

namespace tomovis {

class MeshComponent : public ObjectComponent {
   public:
    MeshComponent(SceneObject& object, int scene_id);
    ~MeshComponent();

    void draw(glm::mat4 world_to_screen) override;
    std::string identifier() const override { return "mesh"; }

    void tick(float time_elapsed) override;
    void describe() override;

   private:
    SceneObject& object_;
    int scene_id_;

    const aiScene* scene_;

    bool show_ = false;
};

}  // namespace tomovis
