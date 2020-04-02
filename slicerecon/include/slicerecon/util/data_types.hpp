#pragma once

#include <array>
#include <utility>
#include <variant>
#include <vector>

namespace slicerecon {

/**
 * The mode that the reconstructor is in. Continuous corresponds to a sliding-
 * window. Alternating is the default.
 * @see https://github.com/cicwi/SliceRecon/issues/4
 */
enum class mode { alternating, continuous };

struct paganin_settings {
    float pixel_size = 1.0f;
    float lambda = 1.23984193e-9f;
    float delta = 1.0e-8f;
    float beta = 1.0e-10f;
    float distance = 40.0f;
};

struct settings {
    int32_t slice_size = 512;
    int32_t preview_size = 128;
    int32_t group_size = 32;
    int32_t filter_cores = 8;
    int32_t darks = 0;
    int32_t flats = 0;
    mode reconstruction_mode = mode::alternating;
    bool already_linear = true;
    bool retrieve_phase = false;
    bool tilt_axis = false;
    paganin_settings paganin = {};
    bool gaussian_pass = false;
    std::string filter = "shepp-logan";
};

namespace acquisition {

/**
 * Parameters that define a simple single-axis circular geometry.
 */
struct geometry {
    int32_t rows = 0;
    int32_t cols = 0;
    int32_t proj_count = 0;
    std::vector<float> angles = {};
    bool parallel = false;
    bool vec_geometry = false;
    std::array<float, 2> detector_size = {0.0f, 0.0f};
    std::array<float, 3> volume_min_point = {0.0f, 0.0f, 0.0f};
    std::array<float, 3> volume_max_point = {1.0f, 1.0f, 1.0f};
    // for cone beam
    float source_origin = 0.0f;
    float origin_det = 0.0f;
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
