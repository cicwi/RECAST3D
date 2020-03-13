#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include <GL/gl3w.h>

namespace tomovis {

static std::map<std::string,
                std::array<std::vector<std::pair<double, double>>, 3>>
    color_data = {{"bone",
                   {{{
                         {0.0, 0.0}, {0.746032, 0.652778}, {1.0, 1.0},
                     },
                     {
                         {0.0, 0.0},
                         {0.365079, 0.319444},
                         {0.746032, 0.777778},
                         {1.0, 1.0},
                     },
                     {
                         {0.0, 0.0}, {0.365079, 0.444444}, {1.0, 1.0},
                     }}}},
                  {"gray",
                   {{{
                         {0.0, 0.0}, {1.0, 1.0},
                     },
                     {
                         {0.0, 0.0}, {1.0, 1.0},
                     },
                     {
                         {0.0, 0.0}, {1.0, 1.0},
                     }}}}};

void set_colormap(std::string name, GLuint colormap_texture_id) {
    constexpr int samples = 100;

    if (color_data.find(name) == color_data.end()) {
        std::cout << "ERROR: colorscheme '" << name << "' not found\n";
    }

    auto cm_data = color_data.find(name)->second;
    auto interpolate = [](
        double z, std::vector<std::pair<double, double>>& xys) -> double {
        for (int i = 1; i < (int)xys.size(); ++i) {
            if (z > xys[i].first) continue;

            auto val =
                xys[i - 1].second +
                ((z - xys[i - 1].first) / (xys[i].first - xys[i - 1].first)) *
                    (xys[i].second - xys[i - 1].second);

            return val;
        }

        return 0.0f;
    };

    unsigned char image[samples * 3];
    for (int j = 0; j < samples; ++j) {
        for (int i = 0; i < 3; ++i) {
            double intensity = (double)j / samples;
            image[j * 3 + i] =
                (unsigned char)(255 * interpolate(intensity, cm_data[i]));
        }
    }

    glBindTexture(GL_TEXTURE_1D, colormap_texture_id);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, samples, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 image);
    glGenerateMipmap(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint generate_colormap_texture(std::string name) {
    GLuint colormap_texture_id;
    glGenTextures(1, &colormap_texture_id);
    set_colormap(name, colormap_texture_id);
    return colormap_texture_id;
}

}  // namespace tomovis
