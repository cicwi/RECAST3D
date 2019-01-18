#pragma once

#include <complex>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

extern "C" {
#include <fftw3.h>
}

#ifndef ASTRA_CUDA
#define ASTRA_CUDA
#endif

#include "astra/ConeProjectionGeometry3D.h"
#include "astra/ConeVecProjectionGeometry3D.h"
#include "astra/CudaBackProjectionAlgorithm3D.h"
#include "astra/CudaProjector3D.h"
#include "astra/Float32Data3DGPU.h"
#include "astra/Float32ProjectionData3DGPU.h"
#include "astra/Float32VolumeData3DGPU.h"
#include "astra/ParallelProjectionGeometry3D.h"
#include "astra/ParallelVecProjectionGeometry3D.h"
#include "astra/VolumeGeometry3D.h"

#include "bulk/backends/thread/thread.hpp"
#include "bulk/bulk.hpp"

#include "../util/data_types.hpp"
#include "../util/exceptions.hpp"
#include "../util/log.hpp"
#include "../util/processing.hpp"
#include "helpers.hpp"

namespace slicerecon {

class reconstructor;

class listener {
  public:
    virtual void notify(reconstructor& recon) = 0;
};

template <typename T>
void minmaxoutput(std::string name, const std::vector<T>& xs) {
    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info << name
                          << " (" << *std::min_element(xs.begin(), xs.end())
                          << ", " << *std::max_element(xs.begin(), xs.end())
                          << ")" << slicerecon::util::end_log;
}

namespace detail {

class solver {
  public:
    solver(settings parameters, acquisition::geometry geometry);
    virtual ~solver();

    virtual slice_data reconstruct_slice(orientation x, int buffer_idx) = 0;
    virtual void reconstruct_preview(std::vector<float>& preview_buffer,
                                     int buffer_idx) = 0;

    auto proj_data(int index) { return proj_datas_[index].get(); }

  protected:
    settings parameters_;
    acquisition::geometry geometry_;

    std::unique_ptr<astra::CVolumeGeometry3D> vol_geom_;
    astraCUDA3d::MemHandle3D vol_handle_;
    std::unique_ptr<astra::CFloat32VolumeData3DGPU> vol_data_;
    std::unique_ptr<astra::CCudaProjector3D> projector_;

    std::unique_ptr<astra::CVolumeGeometry3D> vol_geom_small_;
    astraCUDA3d::MemHandle3D vol_handle_small_;
    std::unique_ptr<astra::CFloat32VolumeData3DGPU> vol_data_small_;
    std::vector<std::unique_ptr<astra::CCudaBackProjectionAlgorithm3D>>
        algs_small_;

    std::vector<std::unique_ptr<astra::CFloat32ProjectionData3DGPU>>
        proj_datas_;
    std::vector<std::unique_ptr<astra::CCudaBackProjectionAlgorithm3D>> algs_;
    std::vector<astraCUDA3d::MemHandle3D> proj_handles_;
};

class parallel_beam_solver : public solver {
  public:
    parallel_beam_solver(settings parameters, acquisition::geometry geometry);
    // FIXME ~solver clean up

    slice_data reconstruct_slice(orientation x, int buffer_idx) override;
    void reconstruct_preview(std::vector<float>& preview_buffer,
                             int buffer_idx) override;

  private:
    // Parallel specific stuff
    std::unique_ptr<astra::CParallelVecProjectionGeometry3D> proj_geom_;
    std::unique_ptr<astra::CParallelVecProjectionGeometry3D> proj_geom_small_;
    std::vector<astra::SPar3DProjection> vectors_;
    std::vector<astra::SPar3DProjection> vec_buf_;
};

class cone_beam_solver : public solver {
  public:
    cone_beam_solver(settings parameters, acquisition::geometry geometry);
    // FIXME ~solver clean up

    slice_data reconstruct_slice(orientation x, int buffer_idx) override;
    void reconstruct_preview(std::vector<float>& preview_buffer,
                             int buffer_idx) override;
    std::vector<float> fdk_weights();

  private:
    // Cone specific stuff
    std::unique_ptr<astra::CConeVecProjectionGeometry3D> proj_geom_;
    std::unique_ptr<astra::CConeVecProjectionGeometry3D> proj_geom_small_;
    std::vector<astra::SConeProjection> vectors_;
    std::vector<astra::SConeProjection> vec_buf_;
};

} // namespace detail

class reconstructor {
  public:
    reconstructor(settings parameters);
    void initialize(acquisition::geometry geom);

    void add_listener(listener* l) { listeners_.push_back(l); }

    /**
     * Push a projection into the reconstruction server.
     *
     * @param k
     * @param proj_idx Projection index
     * @param shape
     * @param data
     */
    void push_projection(proj_kind k, int32_t proj_idx, std::array<int32_t, 2> shape,
            char* data) {
        auto p = parameters_;
        bool is_alt = p.reconstruction_mode == alternating;
        int ue = p.update_every;
        int gs = p.group_size;

        if (!initialized_) {
            slicerecon::util::log
                << LOG_FILE << slicerecon::util::lvl::error
                << "Pushing projection into uninitialized reconstructor"
                << slicerecon::util::end_log;
            return;
        }

        if (shape[0] * shape[1] != pixels_) {
            util::log << LOG_FILE << util::lvl::warning
                      << "Received projection of wrong shape [(" << shape[0]
                      << " x " << shape[1] << ") != " << pixels_
                      << util::end_log;
            throw server_error(
                "Received projection has a different shape than the one set by "
                "the acquisition geometry");
        }

        switch (k) {
            case proj_kind::standard: {
                // check if we received a (new) batch of darks/flats
                if (received_flats_ >= p.darks + p.flats &&
                    p.darks > 0 && p.flats > 0) {
                    compute_flatfielding_();
                    received_flats_ = 0;
                }

                auto rel_proj_idx = proj_idx % ue;
                bool full_group =  rel_proj_idx % gs == gs - 1;
                bool buffer_end_reached = proj_idx % ue == ue - 1;

                // buffer incoming
                memcpy(&buffer_[rel_proj_idx * pixels_], data, sizeof(float) * pixels_);

                // see if some processing/uploading needs to be done
                if (full_group || buffer_end_reached) {

                    // find processing range in the data buffer
                    auto begin_in_buffer = rel_proj_idx - (rel_proj_idx % gs); // start idx of this group

                    // start of threaded processing
                    process_(begin_in_buffer, rel_proj_idx);

                    if (buffer_end_reached) {
                        // copy data from buffer into sino_buffer
                        transpose_into_sino_(0, ue - 1);

                        auto begin_wrt_geom = (update_count_ * ue) % geom_.proj_count;
                        auto end_wrt_geom = (begin_wrt_geom + ue - 1) % geom_.proj_count;
                        bool use_gpu_lock = !is_alt;
                        int gpu_buffer_idx = is_alt ? 1 - active_gpu_buffer : 0;

                        if (end_wrt_geom > begin_wrt_geom) {
                            upload_sino_buffer_(begin_wrt_geom, end_wrt_geom, 0, gpu_buffer_idx, use_gpu_lock);
                        } else {
                            // we have gone around in the geometry
                            upload_sino_buffer_(begin_wrt_geom, geom_.proj_count - 1, 0, gpu_buffer_idx, use_gpu_lock);
                            upload_sino_buffer_(0, end_wrt_geom, geom_.proj_count - begin_wrt_geom - 1,
                                                gpu_buffer_idx, use_gpu_lock);
                        }

                        // let the reconstructor know that now the (other) GPU buffer is ready for reconstruction
                        active_gpu_buffer = gpu_buffer_idx;

                        refresh_data_();
                        update_count_++;
                    }
                }

                break;
            }
            case proj_kind::dark: {
                memcpy(&all_darks_[proj_idx * pixels_], data, sizeof(float) * pixels_);
                received_flats_++;
                break;
            }
            case proj_kind::light: {
                memcpy(&all_flats_[proj_idx * pixels_], data, sizeof(float) * pixels_);
                received_flats_++;
                break;
            }
            default:
                break;
        }
    }

    slice_data reconstruct_slice(orientation x) {
        // the lock is supposed to be always open if reconstruction mode == alternating
        std::lock_guard<std::mutex> guard(gpu_mutex_);

        if (!initialized_) {
            return {{1, 1}, {0.0f}};
        }

        return alg_->reconstruct_slice(x, active_gpu_buffer);
    }

    std::vector<float>& preview_data() { return small_volume_buffer_; }
    settings parameters() { return parameters_; }
    acquisition::geometry geometry() { return geom_; }
    bool initialized() const { return initialized_; }

    void set_scan_settings(int darks, int flats, bool already_linear) {
        parameters_.darks = darks;
        parameters_.flats = flats;
        parameters_.already_linear = already_linear;
    }

  private:
    std::vector<float> average_(std::vector<float> all) {
        auto result = std::vector<float>(pixels_);
        auto samples = all.size() / pixels_;
        for (int i = 0; i < pixels_; ++i) {
            float total = 0.0f;
            for (auto j = 0u; j < samples; ++j) {
                total += all[pixels_ * j + i];
            }
            result[i] = total / samples;
        }
        return result;
    }

    void compute_flatfielding_() {
        slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                              << "Computing reciprocal for flat fielding"
                              << slicerecon::util::end_log;

        // 1) average dark
        auto dark = average_(all_darks_);
        // 2) average flats
        auto light = average_(all_flats_);

        // 3) compute reciprocal
        for (int i = 0; i < geom_.rows * geom_.cols; ++i) {
            if (dark[i] == light[i]) {
                flat_fielder_[i] = 1.0f;
            } else {
                flat_fielder_[i] = 1.0f / (light[i] - dark[i]);
            }
        }

        dark_ = dark;
    }

    void process_(int proj_id_begin, int proj_id_end);

    void upload_sino_buffer_(int proj_id_begin, int proj_id_end, int buffer_begin, int buffer_idx, bool lock_gpu = false);

    void transpose_into_sino_(int proj_offset, int group_size);

    void refresh_data_();

    std::vector<float> all_darks_;
    std::vector<float> all_flats_;
    std::vector<float> dark_;
    std::vector<float> flat_fielder_;
    std::vector<float> buffer_;

    int active_gpu_buffer = 0;

    int32_t pixels_ = -1;
    int32_t received_flats_ = 0;

    std::unique_ptr<detail::solver> alg_;

    acquisition::geometry geom_;
    settings parameters_;

    int update_count_ = 0;
    std::vector<float> small_volume_buffer_;

    std::vector<float> sino_buffer_;

    std::vector<listener*> listeners_;
    bool initialized_ = false;

    std::unique_ptr<util::ProjectionProcessor> projection_processor_;
    fftwf_plan fft_plan_;
    fftwf_plan ffti_plan_;
    fftwf_plan fft2d_plan_;
    std::vector<std::vector<std::complex<float>>> freq_buffer_;
    std::vector<float> filter_;
    std::vector<float> paganin_filter_;

    std::mutex gpu_mutex_;
};

} // namespace slicerecon
