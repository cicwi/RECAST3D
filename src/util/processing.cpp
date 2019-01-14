#include <bulk/bulk.hpp>

#include <algorithm>
#include <cmath>
#include <complex>

#include "slicerecon/util/processing.hpp"

namespace slicerecon::util {

void process_projection(bulk::world& world, int rows, int cols, float* data,
                        const float* dark, const float* reciproc,
                        const std::vector<float>& filter, int proj_id_min,
                        int proj_id_max, bool weigh,
                        const std::vector<float>& fdk_weights, bool neglog,
                        fftwf_plan plan, fftwf_plan iplan,
                        std::vector<std::complex<float>>& freq_buffer,
                        bool retrieve_phase, const std::vector<float>&,
                        fftwf_plan plan2d) {
    // divide work by rows
    int s = world.rank();
    int p = world.active_processors();
    int block_size = ((rows - 1) / p) + 1;
    int first_row = s * block_size;
    int final_row = std::min((s + 1) * block_size, rows);

    int count = proj_id_max - proj_id_min + 1;

    for (int proj = 0; proj < count; ++proj) {
        auto offset = proj * rows * cols;
        for (auto r = first_row; r < final_row; ++r) {
            int index = r * cols;
            for (auto c = 0; c < cols; ++c) {
                if (neglog) {
                    data[offset + index] =
                        (data[offset + index] - dark[index]) * reciproc[index];
                    data[offset + index] =
                        data[offset + index] <= 0.0f
                            ? 0.0f
                            : -std::log(data[offset + index]);
                }
                if (weigh) {
                    data[offset + index] *= fdk_weights[offset + index];
                }
                index++;
            }

            // filter the row
            fftwf_execute_dft_r2c(
                plan, &data[offset + r * cols],
                reinterpret_cast<fftwf_complex*>(&freq_buffer[0]));

            for (int i = 0; i < cols; ++i) {
                freq_buffer[i] *= filter[i];
            }

            fftwf_execute_dft_c2r(
                iplan, reinterpret_cast<fftwf_complex*>(&freq_buffer[0]),
                &data[offset + r * cols]);
        }
    }

    world.barrier();
}

namespace filter {

std::vector<float> ram_lak(int cols) {
    auto result = std::vector<float>(cols);
    auto mid = (cols + 1) / 2;
    for (int i = 0; i < mid; ++i) {
        result[i] = i;
    }
    for (int j = mid; j < cols; ++j) {
        result[j] = 2 * mid - j;
    }
    return result;
}

std::vector<float> shepp_logan(int cols) {
    auto result = std::vector<float>(cols);
    auto mid = (cols + 1) / 2;

    auto filter_weight = [=](auto i) {
        auto norm_freq = (i / (float)mid);
        return norm_freq * std::sin(M_PI * norm_freq) / (M_PI * norm_freq);
    };

    for (int i = 1; i < mid; ++i) {
        result[i] = filter_weight(i);
    }
    for (int j = mid; j < cols; ++j) {
        result[j] = filter_weight(2 * mid - j);
    }
    return result;
}

std::vector<float> gaussian(int cols, float sigma) {
    auto result = std::vector<float>(cols);
    auto mid = (cols + 1) / 2;

    auto filter_weight = [=](auto i) {
        auto norm_freq = (i / (float)mid);
        return std::exp(-(norm_freq * norm_freq) / (2.0f * sigma * sigma));
    };

    for (int i = 1; i < mid; ++i) {
        result[i] = filter_weight(i);
    }
    for (int j = mid; j < cols; ++j) {
        result[j] = filter_weight(2 * mid - j);
    }
    return result;
}

std::vector<float> paganin(int rows, int cols, float pixel_size, float lambda,
                           float delta, float beta, float distance) {
    (void)pixel_size;
    (void)lambda;
    (void)delta;
    (void)beta;
    (void)distance;
    //     delta_x = pixelSize/(2*numpy.pi); delta_y = delta_x #quadratic
    //     pixels, pixelSize in meter
    // #delta_x, delta_y remain after padding (fix pixelsize), fftOfImage.shape
    // takes care of additional (padded) pixels #The fftfreq(....) is necessary
    // to give the proper k space units (meter^{-1}), since lamda and distance
    // are as well in meters, #otherwise the term
    // 'distance*lamda*delta*k_squared' would have inconsistent units. (The k
    // units would be pixelsize^{-1}) k_x =
    // numpy.fft.fftfreq(fftOfImage.shape[1], d=delta_x); k_y =
    // numpy.fft.fftfreq(fftOfImage.shape[0], d=delta_y) k_x_grid, k_y_grid =
    // numpy.meshgrid(k_x, k_y) k_squared = k_x_grid**2 + k_y_grid**2
    // kSpaceFilter = 1.0/(1.0 +
    // distance*lamda*delta*k_squared/(4*numpy.pi*beta))

    return std::vector<float>(rows * cols);
}

} // namespace filter

} // namespace slicerecon::util
