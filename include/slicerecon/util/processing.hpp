#pragma once

#include "data_types.hpp"

#include <cmath>
#include <complex>
#include <vector>

#include <unsupported/Eigen/FFT>

namespace bulk {
class world;
} // namespace bulk

namespace slicerecon::util {

void process_projection(bulk::world& world, int rows, int cols, float* data,
                        const float* dark, const float* reciproc,
                        const std::vector<float>& filter, int proj_id_min,
                        int proj_id_max, bool weigh,
                        const std::vector<float>& fdk_weights, bool neglog);

namespace filter {

std::vector<float> ram_lak(int cols);
std::vector<float> shepp_logan(int cols);

} // namespace filter

} // namespace slicerecon::util
