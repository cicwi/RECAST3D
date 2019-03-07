#include <bulk/bulk.hpp>

#include <algorithm>
#include <cmath>
#include <complex>

#include "slicerecon/util/bench.hpp"
#include "slicerecon/util/processing.hpp"

namespace slicerecon::util {

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

namespace detail {

void Flatfielder::apply(Projection proj) const {
    for (int i = 0; i < proj.rows * proj.cols; ++i) {
        proj.data[i] = (proj.data[i] - dark.data[i]) * reciproc.data[i];
    }
}

void Neglogger::apply(Projection proj) const {
    for (int i = 0; i < proj.rows * proj.cols; ++i) {
        proj.data[i] = proj.data[i] <= 0.0f ? 0.0f : -std::log(proj.data[i]);
    }
}

void FDKScaler::apply(Projection proj, int proj_idx) const {
    auto offset = proj_idx * proj.cols * proj.rows;
    for (int i = 0; i < proj.rows * proj.cols; ++i) {
        proj.data[i] *= weights[offset + i];
    }
}

Paganin::Paganin(settings parameters, acquisition::geometry geom, float* data) {
    proj_freq_buffer_ = std::vector<std::vector<std::complex<float>>>(
        parameters.filter_cores,
        std::vector<std::complex<float>>(geom.cols * geom.rows));
    fft2d_plan_ = fftwf_plan_dft_r2c_2d(
        geom.cols, geom.rows, data,
        reinterpret_cast<fftwf_complex*>(&proj_freq_buffer_[0][0]),
        FFTW_ESTIMATE);
    ffti2d_plan_ = fftwf_plan_dft_c2r_2d(
        geom.cols, geom.rows,
        reinterpret_cast<fftwf_complex*>(&proj_freq_buffer_[0][0]), data,
        FFTW_ESTIMATE);
    paganin_filter_ = util::filter::paganin(
        geom.rows, geom.cols, parameters.paganin.pixel_size,
        parameters.paganin.lambda, parameters.paganin.delta,
        parameters.paganin.beta, parameters.paganin.distance);

    paganin_ = parameters.paganin;
}

void Paganin::apply(Projection proj, int s) {
    // take fft of proj
    fftwf_execute_dft_r2c(
        fft2d_plan_, proj.data,
        reinterpret_cast<fftwf_complex*>(&proj_freq_buffer_[s][0]));

    // filter the proj in 2D
    for (int i = 0; i < proj.rows * proj.cols; ++i) {
        proj_freq_buffer_[s][i] *= paganin_filter_[i];
    }

    // ifft the proj
    fftwf_execute_dft_c2r(
        ffti2d_plan_,
        reinterpret_cast<fftwf_complex*>(&proj_freq_buffer_[s][0]), proj.data);

    // log and scale
    for (int i = 0; i < proj.rows * proj.cols; ++i) {
        proj.data[i] = proj.data[i] <= 0.0f ? 0.0f : -std::log(proj.data[i]);
        proj.data[i] *= paganin_.lambda / (4.0 * M_PI * paganin_.beta);
    }
}

Filterer::Filterer(settings parameters, acquisition::geometry geom,
                   float* data) {
    freq_buffer_ = std::vector<std::vector<std::complex<float>>>(
        parameters.filter_cores, std::vector<std::complex<float>>(geom.cols));
    fft_plan_ = fftwf_plan_dft_r2c_1d(
        geom.cols, data, reinterpret_cast<fftwf_complex*>(&freq_buffer_[0][0]),
        FFTW_ESTIMATE);
    ffti_plan_ = fftwf_plan_dft_c2r_1d(
        geom.cols, reinterpret_cast<fftwf_complex*>(&freq_buffer_[0][0]), data,
        FFTW_ESTIMATE);

    // TODO allow making a choice, optional low pass like below
    filter_ = util::filter::shepp_logan(geom.cols);
    auto filter_lowpass = util::filter::gaussian(geom.cols, 0.06f);
     for (int i = 0; i < geom.cols; ++i) {
        filter_[i] *= filter_lowpass[i];
    }

}

void Filterer::apply(Projection proj, int s) {
    // filter the rows
    for (int row = 0; row < proj.rows; ++row) {
        fftwf_execute_dft_r2c(
            fft_plan_, &proj.data[row * proj.cols],
            reinterpret_cast<fftwf_complex*>(&freq_buffer_[s][0]));

        for (int i = 0; i < proj.cols; ++i) {
            freq_buffer_[s][i] *= filter_[i];
        }

        fftwf_execute_dft_c2r(
            ffti_plan_, reinterpret_cast<fftwf_complex*>(&freq_buffer_[s][0]),
            &proj.data[row * proj.cols]);
    }
}

} // namespace detail

void ProjectionProcessor::process(float* data, int proj_count) {
    auto dt = bulk::util::timer();
    env_.spawn(param_.filter_cores, [&](auto& world) {
        auto s = world.rank();
        auto p = world.active_processors();
        auto pixels = geom_.rows * geom_.cols;

        // we parallelize over projections, and apply the necessary
        // transformations
        for (auto proj_idx = s; proj_idx < proj_count; proj_idx += p) {
            auto proj = detail::Projection{&data[proj_idx * pixels], geom_.rows,
                                           geom_.cols};
            if (flatfielder) {
                flatfielder->apply(proj);
            }
            if (paganin) {
                paganin->apply(proj, world.rank());
            } else if (neglog) {
                neglog->apply(proj);
            }
            if (filterer) {
                filterer->apply(proj, world.rank());
            }
            if (fdk_scale) {
                fdk_scale->apply(proj, proj_idx);
            }
        }

        world.barrier();
    });
    bench.insert("process per proj", dt.get() / proj_count);
}

} // namespace slicerecon::util
