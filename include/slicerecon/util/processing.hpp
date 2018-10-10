#pragma once

#include <vector>
#include <unsupported/Eigen/FFT>

namespace bulk {
class world;
} // namespace bulk

namespace slicerecon {

void process_projection(bulk::world& world, int rows, int cols, float* data, const float* dark,
                        const float* reciproc, const std::vector<float>& filter);

} // namespace slicerecon
