#include "graphics/scene_object_2d.hpp"


namespace tomovis {

SceneObject2D::SceneObject2D() : SceneObject() {
    unsigned char image[size_ * size_ * 3];
    for (int i = 0; i < size_ * size_ * 3; ++i) {
        image[i] = (i % 4 == 0) ?  255 : 0;
    }

    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size_, size_, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

SceneObject2D::~SceneObject2D() {}

void SceneObject2D::draw() {
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    SceneObject::draw();
}

} // namespace tomovis
