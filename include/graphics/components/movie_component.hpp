#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/scene_object.hpp"
#include "graphics/shader_program.hpp"
#include "object_component.hpp"

struct aiScene;

namespace tomovis {

class MovieComponent : public ObjectComponent {
   public:
    MovieComponent(SceneObject& object, int scene_id);
    ~MovieComponent();

    void draw(glm::mat4 world_to_screen) const override;
    std::string identifier() const override { return "movie"; }

    void tick(float time_elapsed) override;
    void describe() override;

   private:
    SceneObject& object_;
    int scene_id_;

    const aiScene* scene_;
};

}  // namespace tomovis
