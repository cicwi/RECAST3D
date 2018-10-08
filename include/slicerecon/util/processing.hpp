#pragma once

#include <vector>
#include <unsupported/Eigen/FFT>

namespace bulk {
class world;
} // namespace bulk

namespace slicerecon {

void process_projection(bulk::world& world, int rows, int cols, float* data, const float* dark,
                        const float* reciproc, const std::vector<float>& filter);

void process_projection_seq(int rows, int cols, float* data, const float* dark,
                            const float* reciproc, Eigen::FFT<float>& fft,
                            std::vector<std::complex<float>>& freq_buffer,
                            const std::vector<float>& filter);

} // namespace slicerecon
