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

/// This listener construction allows the visualization server and the
/// reconstructor to talk to each other anonymously, but it is perhaps a bit
/// overengineered and unclear.
class listener {
  public:
    virtual void notify(reconstructor& recon) = 0;
    virtual void register_parameter(
        std::string parameter_name,
        std::variant<float, std::vector<std::string>, bool> value) = 0;

    void parameter_changed(std::string name,
                           std::variant<float, std::string, bool> value);

  private:
    friend reconstructor;
    void register_(reconstructor* recon) { reconstructor_ = recon; }
    reconstructor* reconstructor_ = nullptr;
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

    // returns true if we want to trigger a re-reconstruction
    virtual bool
    parameter_changed(std::string parameter,
                      std::variant<float, std::string, bool> value) {
        (void)parameter;
        (void)value;
        return false;
    }

    virtual std::vector<std::pair<
        std::string, std::variant<float, std::vector<std::string>, bool>>>
    parameters() {
        return {};
    }

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

    bool
    parameter_changed(std::string parameter,
                      std::variant<float, std::string, bool> value) override;
    std::vector<std::pair<std::string,
                          std::variant<float, std::vector<std::string>, bool>>>
    parameters() override;

  private:
    // Parallel specific stuff
    std::unique_ptr<astra::CParallelVecProjectionGeometry3D> proj_geom_;
    std::unique_ptr<astra::CParallelVecProjectionGeometry3D> proj_geom_small_;
    std::vector<astra::SPar3DProjection> vectors_;
    std::vector<astra::SPar3DProjection> original_vectors_;
    std::vector<astra::SPar3DProjection> vec_buf_;
    float tilt_translate_ = 0.0f;
    float tilt_rotate_ = 0.0f;
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

    // Disallow copies
    reconstructor(const reconstructor&) = delete;

    void initialize(acquisition::geometry geom);

    void add_listener(listener* l) {
        listeners_.push_back(l);
        l->register_(this);

        for (auto [k, v] : float_parameters_) {
            l->register_parameter(k, *v);
        }
        for (auto [k, v] : bool_parameters_) {
            l->register_parameter(k, *v);
        }
    }

    /**
     * Push a projection into the reconstruction server.
     *
     * @param k
     * @param proj_idx Projection index
     * @param shape
     * @param data
     */
    void push_projection(proj_kind k, int32_t proj_idx,
                         std::array<int32_t, 2> shape, char* data) {
        auto p = parameters_;
        int ue = update_every_;
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
                      << "Received projection of wrong shape (" << shape[0]
                      << " x " << shape[1] << ") != " << pixels_
                      << util::end_log;
            throw server_error(
                "Received projection has a different shape than the one set by "
                "the acquisition geometry");
        }

        switch (k) {
        case proj_kind::standard: {
            // check if we received a (new) batch of darks/flats
            if (received_flats_ >= p.darks + p.flats && p.darks > 0 &&
                p.flats > 0) {
                compute_flatfielding_();
                received_flats_ = 0;
            }

            auto rel_proj_idx = proj_idx % ue;
            bool full_group = rel_proj_idx % gs == gs - 1;
            bool buffer_end_reached = proj_idx % ue == ue - 1;

            // buffer incoming
            memcpy(&buffer_[rel_proj_idx * pixels_], data,
                   sizeof(float) * pixels_);

            // see if some processing needs to be done
            if (full_group || buffer_end_reached) {
                // find processing range in the data buffer
                auto begin_in_buffer =
                    rel_proj_idx -
                    (rel_proj_idx % gs); // starting idx of this group
                process_(begin_in_buffer, rel_proj_idx);
            }

            // see if uploading needs to be done
            if (buffer_end_reached) {
                // copy data from buffer into sino_buffer

                if (p.reconstruction_mode == mode::alternating) {
                    transpose_into_sino_(0, ue - 1);

                    // let the reconstructor know that now the (other) GPU
                    // buffer is ready for reconstruction
                    active_gpu_buffer_index_ = 1 - active_gpu_buffer_index_;
                    bool use_gpu_lock = false;

                    upload_sino_buffer_(0, ue - 1, active_gpu_buffer_index_,
                                        use_gpu_lock);

                } else { // --continuous mode
                    auto begin_wrt_geom =
                        (update_count_ * ue) % geom_.proj_count;
                    auto end_wrt_geom =
                        (begin_wrt_geom + ue - 1) % geom_.proj_count;
                    bool use_gpu_lock = true;
                    int gpu_buffer_idx = 0; // we only have one buffer

                    if (end_wrt_geom > begin_wrt_geom) {
                        transpose_into_sino_(0, ue - 1);
                        upload_sino_buffer_(begin_wrt_geom, end_wrt_geom,
                                            gpu_buffer_idx, use_gpu_lock);
                    } else {
                        transpose_into_sino_(0, geom_.proj_count - 1 -
                                                    begin_wrt_geom);
                        // we have gone around in the geometry
                        upload_sino_buffer_(begin_wrt_geom,
                                            geom_.proj_count - 1,
                                            gpu_buffer_idx, use_gpu_lock);

                        transpose_into_sino_(geom_.proj_count - begin_wrt_geom,
                                             ue - 1);
                        upload_sino_buffer_(0, end_wrt_geom, gpu_buffer_idx,
                                            use_gpu_lock);
                    }

                    update_count_++;
                }

                // update low-quality 3D reconstruction
                refresh_data_();
            }

            break;
        }
        case proj_kind::dark: {
            memcpy(&all_darks_[proj_idx * pixels_], data,
                   sizeof(float) * pixels_);
            received_flats_++;
            break;
        }
        case proj_kind::light: {
            memcpy(&all_flats_[proj_idx * pixels_], data,
                   sizeof(float) * pixels_);
            received_flats_++;
            break;
        }
        default:
            break;
        }
    }

    slice_data reconstruct_slice(orientation x) {
        // the lock is supposed to be always open if reconstruction mode ==
        // alternating
        std::lock_guard<std::mutex> guard(gpu_mutex_);

        if (!initialized_) {
            return {{1, 1}, {0.0f}};
        }

        return alg_->reconstruct_slice(x, active_gpu_buffer_index_);
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

    void parameter_changed(std::string name,
                           std::variant<float, std::string, bool> value) {
        if (alg_) {
            if (alg_->parameter_changed(name, value)) {
                for (auto l : listeners_) {
                    l->notify(*this);
                }
            }
        }

        std::visit(
            [&](auto&& x) {
                std::cout << "Param " << name << " changed to " << x << "\n";
            },
            value);
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

    void upload_sino_buffer_(int proj_id_begin, int proj_id_end, int buffer_idx,
                             bool lock_gpu = false);

    void transpose_into_sino_(int proj_offset, int proj_end);

    void refresh_data_();

    std::vector<float> all_darks_;
    std::vector<float> all_flats_;
    std::vector<float> dark_;
    std::vector<float> flat_fielder_;
    std::vector<float> buffer_;

    int active_gpu_buffer_index_ = 0;
    int update_every_;

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

    std::mutex gpu_mutex_;

    // list of parameters that can be changed from the visualization UI
    // NOTE: the enum parameters are hard coded into the handler
    std::map<std::string, float*> float_parameters_;
    std::map<std::string, bool*> bool_parameters_;
};

} // namespace slicerecon
