#pragma once

#include "data_types.hpp"

#include <cmath>
#include <complex>
#include <vector>

extern "C" {
#include <fftw3.h>
}

#include <bulk/backends/thread/thread.hpp>
#include <bulk/bulk.hpp>

namespace slicerecon::util {

namespace filter {
std::vector<float> ram_lak(int cols);
std::vector<float> shepp_logan(int cols);
std::vector<float> gaussian(int cols, float sigma);
std::vector<float> paganin(int rows, int cols, float pixel_size, float lambda,
                           float delta, float beta, float distance);
} // namespace filter

namespace detail {

struct Projection {
    float* data;
    int rows;
    int cols;
};

struct Flatfielder {
    Projection dark;
    Projection reciproc;

    void apply(Projection proj) const;
};

struct Neglogger {
    void apply(Projection proj) const;
};

struct FDKScaler {
    std::vector<float> weights;
    void apply(Projection proj, int proj_idx) const;
};

struct Paganin {
    Paganin(settings parameters, acquisition::geometry geom, float* data);

    void apply(Projection proj, int s);

  private:
    fftwf_plan fft2d_plan_;
    fftwf_plan ffti2d_plan_;
    std::vector<std::vector<std::complex<float>>> proj_freq_buffer_;
    std::vector<float> paganin_filter_;
    paganin_settings paganin_;
};

class Filterer {
  public:
    Filterer(settings parameters, acquisition::geometry geom, float* data);

    void set_filter(std::vector<float> filter) { filter_ = filter; }

    void apply(Projection proj, int s);

  private:
    std::vector<std::vector<std::complex<float>>> freq_buffer_;
    std::vector<float> filter_;
    fftwf_plan fft_plan_;
    fftwf_plan ffti_plan_;
};

} // namespace detail

class ProjectionProcessor {
  public:
    ProjectionProcessor(settings param, acquisition::geometry geom)
        : param_(param), geom_(geom) {}

    void process(float* data, int proj_count) {
        env_.spawn(param_.filter_cores, [&](auto& world) {
            auto s = world.rank();
            auto p = world.active_processors();
            auto pixels = geom_.rows * geom_.cols;

            // we parallelize over projections, and apply the necessary
            // transformations
            for (auto proj_idx = s; proj_idx < proj_count; proj_idx += p) {
                auto proj = detail::Projection{&data[proj_idx * pixels],
                                               geom_.rows, geom_.cols};
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
    }

    std::unique_ptr<detail::Flatfielder> flatfielder;
    std::unique_ptr<detail::Neglogger> neglog;
    std::unique_ptr<detail::Filterer> filterer;
    std::unique_ptr<detail::Paganin> paganin;
    std::unique_ptr<detail::FDKScaler> fdk_scale;

  private:
    settings param_;
    acquisition::geometry geom_;

    bulk::thread::environment env_;
};

} // namespace slicerecon::util
