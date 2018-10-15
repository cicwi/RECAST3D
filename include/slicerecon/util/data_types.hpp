#pragma once

#include <array>
#include <utility>
#include <variant>
#include <vector>

namespace slicerecon {

namespace acquisition {

/**
 * Parameters that define a simple single-axis circular geometry.
 */
struct geometry {
    int32_t rows;
    int32_t cols;
    int32_t proj_count;
    std::vector<float> angles;
    bool parallel;
    bool vec_geometry;
    // for cone beam
    float source_origin;
    float origin_det;
    std::array<float, 2> detector_size;
};

} // namespace acquisition

/**
 * The orientation is an array of 9 floating point numbers. This corresponds to
 * the way tomopackets defines an orientation.
 */
using orientation = std::array<float, 9>;

/**
 * Slice data is a pair of a size in pixels, and the packed data as tomopackets
 * expects it.
 */
using slice_data = std::pair<std::array<int32_t, 2>, std::vector<float>>;

/**
 * An enum with the different projection kinds, which are dark, light, and
 * standard.
 */
enum class proj_kind : int32_t { dark = 0, light = 1, standard = 2 };

/**
 * The data that defines a projection.
 */
struct projection {
    proj_kind kind;
    int32_t idx;
    std::array<int32_t, 2> size;
    std::vector<float> data;
};

} // namespace slicerecon
