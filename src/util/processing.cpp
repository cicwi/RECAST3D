#include <bulk/bulk.hpp>

#include <algorithm>
#include <cmath>
#include <complex>

#include "slicerecon/util/processing.hpp"

namespace slicerecon::util {

void process_projection(
    bulk::world& world, int rows, int cols, float* data, const float* dark,
    const float* reciproc, const std::vector<float>& filter, int proj_id_min,
    int proj_id_max, bool weigh, const std::vector<float>& fdk_weights,
    bool neglog, fftwf_plan plan, fftwf_plan iplan,
    std::vector<std::complex<float>>& freq_buffer, bool retrieve_phase,
    const std::vector<float>& proj_filter, fftwf_plan plan2d,
    fftwf_plan plan2di, std::vector<std::complex<float>>& proj_freq_buffer,
    float lambda, float beta) {
    // divide work by rows
    int s = world.rank();
    int p = world.active_processors();
    int block_size = ((rows - 1) / p) + 1;
    int first_row = s * block_size;
    int final_row = std::min((s + 1) * block_size, rows);

    int count = proj_id_max - proj_id_min + 1;

    // distribute over the projections now
    if (retrieve_phase) {
        for (int proj = s; proj < count; proj += p) {
            // take fft of proj
            fftwf_execute_dft_r2c(
                plan2d, &data[proj * rows * cols],
                reinterpret_cast<fftwf_complex*>(&proj_freq_buffer[0]));

            // filter the proj in 2D
            for (int i = 0; i < rows * cols; ++i) {
                proj_freq_buffer[i] *= proj_filter[i];
            }

            // ifft the proj
            fftwf_execute_dft_c2r(
                plan2di, reinterpret_cast<fftwf_complex*>(&proj_freq_buffer[0]),
                &data[proj * rows * cols]);

            // ... scaling and neglog done below
        }
    }

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
                    if (retrieve_phase) {
                        data[offset + index] *= lambda / (4.0 * M_PI * beta);
                    }
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
    auto filter = std::vector<float>(rows * cols);

    auto dx = pixel_size / (2.0f * M_PI);
    auto dy = dx;
    auto mid_x = (cols + 1) / 2;
    auto mid_y = (rows + 1) / 2;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // TODO Is this is FFTW convention compared to numpy?
            auto x = i < mid_x ? i : (2 * mid_x - i);
            auto y = j < mid_y ? j : (2 * mid_y - j);
            auto k_x = x * dx;
            auto k_y = y * dy;
            auto k_squared = k_x * k_x + k_y * k_y;
            filter[i * cols + j] = (4.0f * beta * M_PI) / 1.0f +
                                   distance * lambda * delta * k_squared;
        }
    }
    return filter;
}

} // namespace filter

} // namespace slicerecon::util
