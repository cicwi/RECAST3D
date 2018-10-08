#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "../util/data_types.hpp"
#include "helpers.hpp"

#ifndef ASTRA_CUDA
#define ASTRA_CUDA
#endif

#include "astra/CudaBackProjectionAlgorithm3D.h"
#include "astra/CudaProjector3D.h"
#include "astra/Float32Data3DGPU.h"
#include "astra/Float32ProjectionData3DGPU.h"
#include "astra/Float32VolumeData3DGPU.h"
#include "astra/ParallelProjectionGeometry3D.h"
#include "astra/ParallelVecProjectionGeometry3D.h"
#include "astra/VolumeGeometry3D.h"

namespace slicerecon {

namespace detail {

class solver {
  public:
    void reconstruct_slice();
};

class parallel_beam_solver : public solver {
  private:
    //--------------------------------------------------------------------------------
    // ASTRA STUFF
    astraCUDA3d::MemHandle3D proj_handle;
    std::unique_ptr<astra::CFloat32ProjectionData3DGPU> proj_data;
    std::unique_ptr<astra::CVolumeGeometry3D> vol_geom;
    astraCUDA3d::MemHandle3D vol_handle;
    std::unique_ptr<astra::CFloat32VolumeData3DGPU> vol_data;
    std::unique_ptr<astra::CCudaProjector3D> projector;
    std::unique_ptr<astra::CCudaBackProjectionAlgorithm3D> alg;

    std::unique_ptr<astra::CVolumeGeometry3D> vol_geom_small;
    astraCUDA3d::MemHandle3D vol_handle_small;
    std::unique_ptr<astra::CFloat32VolumeData3DGPU> vol_data_small;
    std::vector<std::unique_ptr<astra::CCudaBackProjectionAlgorithm3D>>
        algs_small;

    std::vector<std::unique_ptr<astra::CFloat32ProjectionData3DGPU>> proj_datas;
    std::vector<std::unique_ptr<astra::CCudaBackProjectionAlgorithm3D>> algs;
    std::vector<astraCUDA3d::MemHandle3D> proj_handles;

    // Parallel specific stuff
    std::unique_ptr<astra::CParallelVecProjectionGeometry3D> proj_geom;
    std::unique_ptr<astra::CParallelVecProjectionGeometry3D> proj_geom_small;
    std::vector<astra::SPar3DProjection> vectors;
    std::vector<astra::SPar3DProjection> vec_buf;

    //--------------------------------------------------------------------------------
};

class cone_beam_solver : public solver {};

} // namespace detail

// the stream-independent pool of data, and slice reconstructor
class reconstructor {
  public:
    struct settings {
        int32_t slice_size;
        int32_t preview_size;
        int32_t group_size;
        int32_t filter_cores;
        int32_t darks;
        int32_t lights;
    };

    reconstructor(acquisition::geometry geom, settings parameters);

    // push a projection
    void push_projection(proj_kind k, int32_t idx, std::array<int32_t, 2> shape,
                         char* data) {
        // assert product(shape) == pixels
        switch (k) {
        case proj_kind::standard: {
            // check if we received a (new) batch of darks/flats
            if (received_flats_ > parameters_.darks + parameters_.lights) {
                compute_flatfielding_();
                received_flats_ = 0;
            }

            auto start_idx = start_index_(idx);
            memcpy(&buffer_[write_index_][start_idx], data,
                   sizeof(float) * pixels_);

            if (start_idx == parameters_.group_size - 1) {
                // upload and switch writing idx
                upload_();
                write_index_ = 1 - write_index_;
            }
            break;
        }
        case proj_kind::dark: {
            memcpy(&all_darks_[idx * pixels_], data, sizeof(float) * pixels_);
            received_flats_++;
            break;
        }
        case proj_kind::light: {
            memcpy(&all_lights_[idx * pixels_], data, sizeof(float) * pixels_);
            received_flats_++;
            break;
        }
        }
    }

    slice_data reconstruct_slice(orientation x) {
        // TODO implement TODO have code at TOMCAT-live project for this does
        // this depend on the geometry, I dont think so, maybe we can avoid
        // duplication by a template..
        return {{0, 0}, {}};
    }

  private:
    void initialize_astra_();

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
        // 1) average dark
        auto dark = average_(all_darks_);
        // 2) average flats
        auto light = average_(all_lights_);
        // 3) compute reciprocal
        for (int i = 0; i < rows_ * cols_; ++i) {
            if (dark[i] == light[i]) {
                flat_fielder_[i] = 1.0f;
            } else {
                flat_fielder_[i] = 1.0f / (light[i] - dark[i]);
            }
        }
    }

    void upload_() { /* TODO */
        // 1) filter
        // 2) tranpose sino
        // 3) upload to astra
        // astra::uploadMultipleProjections(proj_datas_[proj_target].get(),
        //                                 sino_buffer.data(), proj_id_min,
        //                                 proj_id_max, false);
    }

    int32_t start_index_(int32_t idx) {
        return (idx % parameters_.group_size) * pixels_;
    }

    std::vector<float> all_darks_;
    std::vector<float> all_lights_;
    std::vector<float> dark_;
    std::vector<float> flat_fielder_;

    std::array<std::vector<float>, 2> buffer_;
    int write_index_ = 0;
    int32_t pixels_ = -1;
    int32_t received_flats_ = 0;

    int32_t rows_ = -1;
    int32_t cols_ = -1;

    std::unique_ptr<detail::solver> alg_;

    acquisition::geometry geom_;
    settings parameters_;

    std::vector<float> filter_;
};

} // namespace slicerecon
