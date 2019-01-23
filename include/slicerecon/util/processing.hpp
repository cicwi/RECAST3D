#pragma once

#include "data_types.hpp"

#include <cmath>
#include <complex>
#include <vector>

extern "C" {
#include <fftw3.h>
}

namespace bulk {
class world;
} // namespace bulk

namespace slicerecon::util {

void process_projection(
    bulk::world& world, int rows, int cols, float* data, const float* dark,
    const float* reciproc, const std::vector<float>& filter, int proj_id_min,
    int proj_id_max, bool weigh, const std::vector<float>& fdk_weights,
    bool neglog, fftwf_plan plan, fftwf_plan iplan,
    std::vector<std::complex<float>>& freq_buffer, bool retrieve_phase,
    const std::vector<float>& proj_filter, fftwf_plan plan2d,
    fftwf_plan plan2di, std::vector<std::complex<float>>& proj_freq_buffer,
    float lambda, float beta);

namespace filter {

std::vector<float> ram_lak(int cols);
std::vector<float> shepp_logan(int cols);
std::vector<float> gaussian(int cols, float sigma);
std::vector<float> paganin(int rows, int cols, float pixel_size, float lambda,
                           float delta, float beta, float distance);

} // namespace filter

} // namespace slicerecon::util
