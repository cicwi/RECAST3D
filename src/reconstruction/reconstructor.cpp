#include <complex>

#include <Eigen/Eigen>

#include "slicerecon/reconstruction/reconstructor.hpp"
#include "slicerecon/util/bench.hpp"
#include "slicerecon/util/processing.hpp"
#include "slicerecon/util/util.hpp"

namespace slicerecon {

void listener::parameter_changed(std::string name,
                                 std::variant<float, std::string, bool> value) {
    reconstructor_->parameter_changed(name, value);
}

namespace detail {

solver::solver(settings parameters, acquisition::geometry geometry)
    : parameters_(parameters), geometry_(geometry) {
    float half_slab_height =
        0.5f * (geometry_.volume_max_point[2] - geometry_.volume_min_point[2]) /
        parameters_.preview_size;
    float mid_z =
        0.5f * (geometry_.volume_max_point[2] + geometry_.volume_min_point[2]);

    // Volume geometry
    vol_geom_ = std::make_unique<astra::CVolumeGeometry3D>(
        parameters_.slice_size, parameters_.slice_size, 1,
        geometry_.volume_min_point[0], geometry_.volume_min_point[1],
        mid_z - half_slab_height, geometry_.volume_max_point[0],
        geometry_.volume_max_point[1], mid_z + half_slab_height);

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Slice vol: " << slicerecon::util::info(*vol_geom_)
                          << slicerecon::util::end_log;

    // Volume data
    vol_handle_ = astraCUDA3d::allocateGPUMemory(parameters_.slice_size,
                                                 parameters_.slice_size, 1,
                                                 astraCUDA3d::INIT_ZERO);
    vol_data_ = std::make_unique<astra::CFloat32VolumeData3DGPU>(
        vol_geom_.get(), vol_handle_);

    // Small preview volume
    vol_geom_small_ = std::make_unique<astra::CVolumeGeometry3D>(
        parameters_.preview_size, parameters_.preview_size,
        parameters_.preview_size, geometry_.volume_min_point[0],
        geometry_.volume_min_point[1], geometry_.volume_min_point[1],
        geometry_.volume_max_point[0], geometry_.volume_max_point[1],
        geometry_.volume_max_point[1]);

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << slicerecon::util::info(*vol_geom_small_)
                          << slicerecon::util::end_log;

    vol_handle_small_ = astraCUDA3d::allocateGPUMemory(
        parameters_.preview_size, parameters_.preview_size,
        parameters_.preview_size, astraCUDA3d::INIT_ZERO);
    vol_data_small_ = std::make_unique<astra::CFloat32VolumeData3DGPU>(
        vol_geom_small_.get(), vol_handle_small_);
}

solver::~solver() {
    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Deconstructing solver and freeing GPU memory"
                          << slicerecon::util::end_log;

    astraCUDA3d::freeGPUMemory(vol_handle_);
    astraCUDA3d::freeGPUMemory(vol_handle_small_);
    for (auto& proj_handle : proj_handles_) {
        astraCUDA3d::freeGPUMemory(proj_handle);
    }
}

parallel_beam_solver::parallel_beam_solver(settings parameters,
                                           acquisition::geometry geometry)
    : solver(parameters, geometry) {
    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Initializing parallel beam solver"
                          << slicerecon::util::end_log;
    if (!geometry_.vec_geometry) {
        // Projection geometry
        auto proj_geom = astra::CParallelProjectionGeometry3D(
            geometry_.proj_count, geometry_.rows, geometry_.cols, 1.0f, 1.0f,
            geometry_.angles.data());

        proj_geom_ = slicerecon::util::proj_to_vec(&proj_geom);

        proj_geom_small_ = slicerecon::util::proj_to_vec(&proj_geom);

    } else {
        auto par_projs =
            slicerecon::util::list_to_par_projections(geometry_.angles);
        proj_geom_ = std::make_unique<astra::CParallelVecProjectionGeometry3D>(
            geometry_.proj_count, geometry_.rows, geometry_.cols,
            par_projs.data());
        proj_geom_small_ =
            std::make_unique<astra::CParallelVecProjectionGeometry3D>(
                geometry_.proj_count, geometry_.rows, geometry_.cols,
                par_projs.data());
    }

    vectors_ = std::vector<astra::SPar3DProjection>(
        proj_geom_->getProjectionVectors(),
        proj_geom_->getProjectionVectors() + geometry_.proj_count);
    original_vectors_ = vectors_;
    vec_buf_ = vectors_;

    auto zeros = std::vector<float>(
        geometry_.proj_count * geometry_.cols * geometry_.rows, 0.0f);

    // Projection data
    int nr_handles =
        parameters.reconstruction_mode == mode::alternating ? 2 : 1;
    for (int i = 0; i < nr_handles; ++i) {
        proj_handles_.push_back(astraCUDA3d::createProjectionArrayHandle(
            zeros.data(), geometry_.cols, geometry_.proj_count,
            geometry_.rows));
        proj_datas_.push_back(
            std::make_unique<astra::CFloat32ProjectionData3DGPU>(
                proj_geom_.get(), proj_handles_[0]));
    }

    // Back projection algorithm, link to previously made objects
    projector_ = std::make_unique<astra::CCudaProjector3D>();
    for (int i = 0; i < nr_handles; ++i) {
        algs_.push_back(std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
            projector_.get(), proj_datas_[i].get(), vol_data_.get()));
        algs_small_.push_back(
            std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
                projector_.get(), proj_datas_[i].get(), vol_data_small_.get()));
    }
}

slice_data parallel_beam_solver::reconstruct_slice(orientation x,
                                                   int buffer_idx) {
    auto dt = util::bench_scope("slice");
    auto k = vol_geom_->getWindowMaxX();

    auto [delta, rot, scale] = util::slice_transform(
        {x[6], x[7], x[8]}, {x[0], x[1], x[2]}, {x[3], x[4], x[5]}, k);

    // From the ASTRA geometry, get the vectors, modify, and reset them
    int i = 0;
    for (auto [rx, ry, rz, dx, dy, dz, pxx, pxy, pxz, pyx, pyy, pyz] :
         vectors_) {
        auto r = Eigen::Vector3f(rx, ry, rz);
        auto d = Eigen::Vector3f(dx, dy, dz);
        auto px = Eigen::Vector3f(pxx, pxy, pxz);
        auto py = Eigen::Vector3f(pyx, pyy, pyz);

        d += 0.5f * (geometry_.cols * px + geometry_.rows * py);
        r = scale.cwiseProduct(rot * r);
        d = scale.cwiseProduct(rot * (d + delta));
        px = scale.cwiseProduct(rot * px);
        py = scale.cwiseProduct(rot * py);
        d -= 0.5f * (geometry_.cols * px + geometry_.rows * py);

        vec_buf_[i] = {r[0],  r[1],  r[2],  d[0],  d[1],  d[2],
                       px[0], px[1], px[2], py[0], py[1], py[2]};
        ++i;
    }

    proj_geom_ = std::make_unique<astra::CParallelVecProjectionGeometry3D>(
        geometry_.proj_count, geometry_.rows, geometry_.cols, vec_buf_.data());

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Reconstructing slice: "
                          << "[" << x[0] << ", " << x[1] << ", " << x[2]
                          << "], "
                          << "[" << x[3] << ", " << x[4] << ", " << x[5]
                          << "], "
                          << "[" << x[6] << ", " << x[7] << ", " << x[8] << "]"
                          << " buffer (" << buffer_idx << ")"
                          << slicerecon::util::end_log;

    proj_datas_[buffer_idx]->changeGeometry(proj_geom_.get());
    algs_[buffer_idx]->run();

    unsigned int n = parameters_.slice_size;
    auto result = std::vector<float>(n * n, 0.0f);
    auto pos = astraCUDA3d::SSubDimensions3D{n, n, 1, n, n, n, 1, 0, 0, 0};
    astraCUDA3d::copyFromGPUMemory(result.data(), vol_handle_, pos);

    return {{(int)n, (int)n}, std::move(result)};
}

void parallel_beam_solver::reconstruct_preview(
    std::vector<float>& preview_buffer, int buffer_idx) {
    auto dt = util::bench_scope("3D preview");

    proj_datas_[buffer_idx]->changeGeometry(proj_geom_small_.get());
    algs_small_[buffer_idx]->run();

    unsigned int n = parameters_.preview_size;
    float factor = (n / (float)geometry_.cols);
    auto pos = astraCUDA3d::SSubDimensions3D{n, n, n, n, n, n, n, 0, 0, 0};
    astraCUDA3d::copyFromGPUMemory(preview_buffer.data(), vol_handle_small_,
                                   pos);

    for (auto& x : preview_buffer) {
        x *= (factor * factor * factor);
    }
}

bool parallel_beam_solver::parameter_changed(
    std::string parameter, std::variant<float, std::string, bool> value) {

    bool tilt_changed = false;
    if (parameter == "tilt angle") {
        tilt_changed = true;
        tilt_rotate_ = std::get<float>(value);
        // TODO rotate geometry vectors
    } else if (parameter == "tilt translate") {
        tilt_changed = true;
        tilt_translate_ = std::get<float>(value);
        // TODO translate geometry vectors
    }

    std::cout << "Rotate to " << tilt_rotate_ << ", translate to"
              << tilt_translate_ << "\n";

    if (tilt_changed) {
        // From the ASTRA geometry, get the vectors, modify, and reset them
        int i = 0;
        for (auto [rx, ry, rz, dx, dy, dz, pxx, pxy, pxz, pyx, pyy, pyz] :
             original_vectors_) {
            auto r = Eigen::Vector3f(rx, ry, rz);
            auto d = Eigen::Vector3f(dx, dy, dz);
            auto px = Eigen::Vector3f(pxx, pxy, pxz);
            auto py = Eigen::Vector3f(pyx, pyy, pyz);

            d += tilt_translate_ * px;

            auto z = px.normalized();
            auto w = py.normalized();
            auto axis = z.cross(w);
            auto rot = Eigen::AngleAxis<float>(tilt_rotate_ * M_PI / 180.0f,
                                               axis.normalized())
                           .matrix();

            px = rot * px;
            py = rot * py;

            vectors_[i] = {r[0],  r[1],  r[2],  d[0],  d[1],  d[2],
                           px[0], px[1], px[2], py[0], py[1], py[2]};
            ++i;
        }

        // TODO if either changed, trigger a new reconstruction. Do we need to
        // do this from reconstructor (since we don't have access to listeners
        // to notify?). Then the order of handling parameters matters.
    }

    return tilt_changed;
}

std::vector<
    std::pair<std::string, std::variant<float, std::vector<std::string>, bool>>>
parallel_beam_solver::parameters() {
    if (!parameters_.tilt_axis) {
        return {};
    }
    return {{"tilt angle", 0.0f}, {"tilt translate", 0.0f}};
}

cone_beam_solver::cone_beam_solver(settings parameters,
                                   acquisition::geometry geometry)
    : solver(parameters, geometry) {
    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Initializing cone beam solver"
                          << slicerecon::util::end_log;

    if (!geometry_.vec_geometry) {
        auto proj_geom = astra::CConeProjectionGeometry3D(
            geometry_.proj_count, geometry_.rows, geometry_.cols,
            geometry_.detector_size[0], geometry_.detector_size[1],
            geometry_.angles.data(), geometry_.source_origin,
            geometry_.origin_det);

        proj_geom_ = slicerecon::util::proj_to_vec(&proj_geom);
        proj_geom_small_ = slicerecon::util::proj_to_vec(&proj_geom);
    } else {
        auto cone_projs = slicerecon::util::list_to_cone_projections(
            geometry_.rows, geometry_.cols, geometry_.angles);
        proj_geom_ = std::make_unique<astra::CConeVecProjectionGeometry3D>(
            geometry_.proj_count, geometry_.rows, geometry_.cols,
            cone_projs.data());

        slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                              << slicerecon::util::info(*proj_geom_)
                              << slicerecon::util::end_log;

        proj_geom_small_ =
            std::make_unique<astra::CConeVecProjectionGeometry3D>(
                geometry_.proj_count, geometry_.rows, geometry_.cols,
                cone_projs.data());
    }

    vectors_ = std::vector<astra::SConeProjection>(
        proj_geom_->getProjectionVectors(),
        proj_geom_->getProjectionVectors() + geometry_.proj_count);
    vec_buf_ = vectors_;

    auto zeros = std::vector<float>(
        geometry_.proj_count * geometry_.cols * geometry_.rows, 0.0f);

    // Projection data
    int nr_handles =
        parameters.reconstruction_mode == mode::alternating ? 2 : 1;
    for (int i = 0; i < nr_handles; ++i) {
        proj_handles_.push_back(astraCUDA3d::createProjectionArrayHandle(
            zeros.data(), geometry_.cols, geometry_.proj_count,
            geometry_.rows));
        proj_datas_.push_back(
            std::make_unique<astra::CFloat32ProjectionData3DGPU>(
                proj_geom_.get(), proj_handles_[0]));
    }

    // Back projection algorithm, link to previously made objects
    projector_ = std::make_unique<astra::CCudaProjector3D>();
    for (int i = 0; i < nr_handles; ++i) {
        algs_.push_back(std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
            projector_.get(), proj_datas_[i].get(), vol_data_.get()));
        algs_small_.push_back(
            std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
                projector_.get(), proj_datas_[i].get(), vol_data_small_.get()));
    }
}

slice_data cone_beam_solver::reconstruct_slice(orientation x, int buffer_idx) {
    auto dt = util::bench_scope("slice");
    auto k = vol_geom_->getWindowMaxX();

    auto [delta, rot, scale] = util::slice_transform(
        {x[6], x[7], x[8]}, {x[0], x[1], x[2]}, {x[3], x[4], x[5]}, k);

    // From the ASTRA geometry, get the vectors, modify, and reset them
    int i = 0;
    for (auto [rx, ry, rz, dx, dy, dz, pxx, pxy, pxz, pyx, pyy, pyz] :
         vectors_) {
        auto s = Eigen::Vector3f(rx, ry, rz);
        auto d = Eigen::Vector3f(dx, dy, dz);
        auto t1 = Eigen::Vector3f(pxx, pxy, pxz);
        auto t2 = Eigen::Vector3f(pyx, pyy, pyz);

        s = scale.cwiseProduct(rot * (s + delta));
        d = scale.cwiseProduct(rot * (d + delta));
        t1 = scale.cwiseProduct(rot * t1);
        t2 = scale.cwiseProduct(rot * t2);

        vec_buf_[i] = {s[0],  s[1],  s[2],  d[0],  d[1],  d[2],
                       t1[0], t1[1], t1[2], t2[0], t2[1], t2[2]};
        ++i;
    }

    proj_geom_ = std::make_unique<astra::CConeVecProjectionGeometry3D>(
        geometry_.proj_count, geometry_.rows, geometry_.cols, vec_buf_.data());

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Reconstructing slice: "
                          << "[" << x[0] << ", " << x[1] << ", " << x[2]
                          << "], "
                          << "[" << x[3] << ", " << x[4] << ", " << x[5]
                          << "], "
                          << "[" << x[6] << ", " << x[7] << ", " << x[8] << "]"
                          << " buffer (" << buffer_idx << ")"
                          << slicerecon::util::end_log;

    proj_datas_[buffer_idx]->changeGeometry(proj_geom_.get());
    algs_[buffer_idx]->run();

    unsigned int n = parameters_.slice_size;
    auto result = std::vector<float>(n * n, 0.0f);
    auto pos = astraCUDA3d::SSubDimensions3D{n, n, 1, n, n, n, 1, 0, 0, 0};
    astraCUDA3d::copyFromGPUMemory(result.data(), vol_handle_, pos);

    return {{(int)n, (int)n}, std::move(result)};
}

void cone_beam_solver::reconstruct_preview(std::vector<float>& preview_buffer,
                                           int buffer_idx) {
    auto dt = util::bench_scope("3D preview");

    proj_datas_[buffer_idx]->changeGeometry(proj_geom_small_.get());
    algs_small_[buffer_idx]->run();

    unsigned int n = parameters_.preview_size;
    auto pos = astraCUDA3d::SSubDimensions3D{n, n, n, n, n, n, n, 0, 0, 0};
    astraCUDA3d::copyFromGPUMemory(preview_buffer.data(), vol_handle_small_,
                                   pos);
}

std::vector<float> cone_beam_solver::fdk_weights() {
    auto result = std::vector<float>(
        geometry_.cols * geometry_.rows * geometry_.proj_count, 0.0f);

    int i = 0;
    for (auto [rx, ry, rz, dx, dy, dz, pxx, pxy, pxz, pyx, pyy, pyz] :
         vectors_) {
        auto s = Eigen::Vector3f(rx, ry, rz);
        auto d = Eigen::Vector3f(dx, dy, dz);
        auto t1 = Eigen::Vector3f(pxx, pxy, pxz);
        auto t2 = Eigen::Vector3f(pyx, pyy, pyz);

        // FIXME uncentered projections
        // rho should be the distance between the source and the detector plane
        // which is not equal to (d - s).norm() for uncentered projections
        auto rho = (d - s).norm();
        for (int r = 0; r < geometry_.rows; ++r) {
            for (int c = 0; c < geometry_.cols; ++c) {
                auto y = d + r * t2 + c * t1;
                auto denum = (y - s).norm();
                result[(i * geometry_.cols * geometry_.rows) +
                       (r * geometry_.cols) + c] = rho / denum;
            }
        }

        ++i;
    }

    return result;
}

} // namespace detail

reconstructor::reconstructor(settings parameters) : parameters_(parameters) {
    float_parameters_["lambda"] = &parameters_.paganin.lambda;
    float_parameters_["delta"] = &parameters_.paganin.delta;
    float_parameters_["beta"] = &parameters_.paganin.beta;
    float_parameters_["distance"] = &parameters_.paganin.distance;
    bool_parameters_["retrieve phase"] = &parameters_.retrieve_phase;
}

void reconstructor::initialize(acquisition::geometry geom) {
    bool reinitializing = (bool)alg_;

    geom_ = geom;

    // init counts
    pixels_ = geom_.cols * geom_.rows;

    // allocate the buffers
    all_flats_.resize(pixels_ * parameters_.flats);
    all_darks_.resize(pixels_ * parameters_.darks);
    dark_.resize(pixels_);
    flat_fielder_.resize(pixels_, 1.0f);
    update_every_ = parameters_.reconstruction_mode == mode::alternating
                        ? geom_.proj_count
                        : parameters_.group_size;

    buffer_.resize((size_t)update_every_ * (size_t)pixels_);
    sino_buffer_.resize((size_t)update_every_ * (size_t)pixels_);

    small_volume_buffer_.resize(parameters_.preview_size *
                                parameters_.preview_size *
                                parameters_.preview_size);

    if (geom_.parallel) {
        // make reconstruction object par
        alg_ =
            std::make_unique<detail::parallel_beam_solver>(parameters_, geom_);
    } else {
        // make reconstruction object cb
        alg_ = std::make_unique<detail::cone_beam_solver>(parameters_, geom_);
    }

    initialized_ = true;

    projection_processor_ =
        std::make_unique<util::ProjectionProcessor>(parameters_, geom_);

    // add flat fielder
    projection_processor_->flatfielder =
        std::make_unique<util::detail::Flatfielder>(util::detail::Flatfielder{
            {dark_.data(), geom_.rows, geom_.cols},
            {flat_fielder_.data(), geom_.rows, geom_.cols}});

    // add neg log
    if (!parameters_.already_linear && !parameters_.retrieve_phase) {
        projection_processor_->neglog =
            std::make_unique<util::detail::Neglogger>(
                util::detail::Neglogger{});
    }

    projection_processor_->filterer = std::make_unique<util::detail::Filterer>(
        util::detail::Filterer{parameters_, geom_, &buffer_[0]});

    if (!geom_.parallel) {
        projection_processor_->fdk_scale =
            std::make_unique<util::detail::FDKScaler>(util::detail::FDKScaler{
                ((detail::cone_beam_solver*)(alg_.get()))->fdk_weights()});
    }

    if (parameters_.retrieve_phase) {
        projection_processor_->paganin =
            std::make_unique<util::detail::Paganin>(
                util::detail::Paganin{parameters_, geom_, &buffer_[0]});
    }

    if (!reinitializing) {
        for (auto [k, v] : alg_->parameters()) {
            for (auto l : listeners_) {
                l->register_parameter(k, v);
            }
        }
    } else {
        slicerecon::util::log
            << LOG_FILE << slicerecon::util::lvl::warning
            << "Reinitializing geometry, not registering parameter controls"
            << slicerecon::util::end_log;
    }
}

/**
 * Copy from a data buffer to a sino buffer, while transposing the data.
 *
 * If an offset is given, it transposes projections [offset, offset+1, ...,
 * proj_end] to the *front* of the sino_buffer_, leaving the remainder of the
 * buffer unused.
 *
 * @param projection_group  Reference to buffered data
 * @param sino_buffer       Reference to the CPU buffer of the projections
 * @param group_size        Number of projections in the group
 *
 */
void reconstructor::transpose_into_sino_(int proj_offset, int proj_end) {
    auto dt = util::bench_scope("Transpose sino");

    // major to minor: [i, j, k]
    // In projection_group we have: [projection_id, rows, cols ]
    // For sinogram we want: [rows, projection_id, cols]

    auto buffer_size = proj_end - proj_offset + 1;

    for (int i = 0; i < geom_.rows; ++i) {
        for (int j = proj_offset; j <= proj_end; ++j) {
            for (int k = 0; k < geom_.cols; ++k) {
                sino_buffer_[i * buffer_size * geom_.cols +
                             (j - proj_offset) * geom_.cols + k] =
                    buffer_[j * geom_.cols * geom_.rows + i * geom_.cols + k];
            }
        }
    }
}

/**
 * In-memory processing the projections [proj_id_begin, ..., proj_id_end]
 *
 * @param proj_id_begin
 * @param proj_id_end
 */
void reconstructor::process_(int proj_id_begin, int proj_id_end) {
    if (!initialized_) {
        return;
    }

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Processing buffer"
                          << " between " << proj_id_begin << "/" << proj_id_end
                          << slicerecon::util::end_log;

    auto data = &buffer_[proj_id_begin * pixels_];
    projection_processor_->process(data, proj_id_end - proj_id_begin + 1);
}

/**
 * Upload the CPU sinogram buffer to ASTRA on the GPU
 *
 * @param proj_id_begin The starting position of the data in the GPU
 * @param proj_id_end The last position of the data in the GPU
 * @param buffer_begin Position of the to-be-uploaded data in the CPU buffer
 * @param buffer_idx Index of the GPU buffer the sinogram data should be
 * uploaded to
 * @param lock_gpu Whether or not to use gpu_mutex_ to block access to the GPU
 */
void reconstructor::upload_sino_buffer_(int proj_id_begin, int proj_id_end,
                                        int buffer_idx, bool lock_gpu) {
    if (!initialized_) {
        return;
    }

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Uploading to buffer (" << active_gpu_buffer_index_
                          << ") between " << proj_id_begin << "/" << proj_id_end
                          << slicerecon::util::end_log;

    // in continuous mode, there is only one data buffer and it needs to be
    // protected since the reconstruction server has access to it too

    {
        auto dt = util::bench_scope("GPU upload");
        if (lock_gpu) {
            std::lock_guard<std::mutex> guard(gpu_mutex_);

            astra::uploadMultipleProjections(alg_->proj_data(buffer_idx),
                                             &sino_buffer_[0], proj_id_begin,
                                             proj_id_end);
        } else {
            astra::uploadMultipleProjections(alg_->proj_data(buffer_idx),
                                             &sino_buffer_[0], proj_id_begin,
                                             proj_id_end);
        }
    }

    // send message to observers that new data is available
    for (auto l : listeners_) {
        l->notify(*this);
    }
}

void reconstructor::refresh_data_() {
    if (!initialized_) {
        return;
    }

    { // lock guard scope
        std::lock_guard<std::mutex> guard(gpu_mutex_);
        alg_->reconstruct_preview(small_volume_buffer_,
                                  active_gpu_buffer_index_);
    } // end lock guard scope

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Reconstructed low-res preview ("
                          << active_gpu_buffer_index_ << ")"
                          << slicerecon::util::end_log;

    // send message to observers that new data is available
    for (auto l : listeners_) {
        l->notify(*this);
    }
}

} // namespace slicerecon
