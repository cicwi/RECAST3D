#include <bulk/bulk.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <unsupported/Eigen/FFT>

#include "slicerecon/util/processing.hpp"

namespace slicerecon::util {

void process_projection(bulk::world& world, int rows, int cols, float* data,
                        const float* dark, const float* reciproc,
                        const std::vector<float>& filter, int proj_id_min,
                        int proj_id_max, bool weigh,
                        const std::vector<float>& fdk_weights, bool neglog) {
    // initialize fft
    auto fft = Eigen::FFT<float>();
    auto freq_buffer = std::vector<std::complex<float>>(cols, 0);

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
            fft.fwd(freq_buffer.data(), &data[offset + r * cols], cols);
            for (int i = 0; i < cols; ++i) {
                freq_buffer[i] *= filter[i];
            }
            fft.inv(&data[offset + r * cols], freq_buffer.data(), cols);
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

} // namespace filter

} // namespace slicerecon::util
