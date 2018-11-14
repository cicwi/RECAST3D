#include "slicerecon/reconstruction/reconstructor.hpp"
#include "slicerecon/util/processing.hpp"
#include "slicerecon/util/util.hpp"

namespace slicerecon {

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
    vec_buf_ = vectors_;

    auto zeros = std::vector<float>(
        geometry_.proj_count * geometry_.cols * geometry_.rows, 0.0f);

    // Projection data
    for (int i = 0; i < 2; ++i) {
        proj_handles_.push_back(astraCUDA3d::createProjectionArrayHandle(
            zeros.data(), geometry_.cols, geometry_.proj_count,
            geometry_.rows));
        proj_datas_.push_back(
            std::make_unique<astra::CFloat32ProjectionData3DGPU>(
                proj_geom_.get(), proj_handles_[0]));
    }
    // Back projection algorithm, link to previously made objects
    projector_ = std::make_unique<astra::CCudaProjector3D>();
    for (int i = 0; i < 2; ++i) {
        algs_.push_back(std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
            projector_.get(), proj_datas_[i].get(), vol_data_.get()));
        algs_small_.push_back(
            std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
                projector_.get(), proj_datas_[i].get(), vol_data_small_.get()));
    }
}

slice_data parallel_beam_solver::reconstruct_slice(orientation x,
                                                   int buffer_idx) {
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
    for (int i = 0; i < 2; ++i) {
        proj_handles_.push_back(astraCUDA3d::createProjectionArrayHandle(
            zeros.data(), geometry_.cols, geometry_.proj_count,
            geometry_.rows));
        proj_datas_.push_back(
            std::make_unique<astra::CFloat32ProjectionData3DGPU>(
                proj_geom_.get(), proj_handles_[0]));
    }

    // Back projection algorithm, link to previously made objects
    projector_ = std::make_unique<astra::CCudaProjector3D>();
    for (int i = 0; i < 2; ++i) {
        algs_.push_back(std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
            projector_.get(), proj_datas_[i].get(), vol_data_.get()));
        algs_small_.push_back(
            std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
                projector_.get(), proj_datas_[i].get(), vol_data_small_.get()));
    }
}

slice_data cone_beam_solver::reconstruct_slice(orientation x, int buffer_idx) {
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

reconstructor::reconstructor(settings parameters) : parameters_(parameters) {}

void reconstructor::initialize(acquisition::geometry geom) {
    geom_ = geom;

    // init counts
    pixels_ = geom_.cols * geom_.rows;
    current_group_ = 0;
    group_count_ = (geom_.proj_count - 1) / parameters_.group_size + 1;

    // allocate the buffers
    all_flats_.resize(pixels_ * parameters_.flats);
    all_darks_.resize(pixels_ * parameters_.darks);
    dark_.resize(pixels_);
    flat_fielder_.resize(pixels_);
    buffer_[0].resize(parameters_.group_size * pixels_);
    buffer_[1].resize(parameters_.group_size * pixels_);
    sino_buffer_.resize(geom_.rows * geom_.cols * parameters_.group_size);
    small_volume_buffer_.resize(parameters_.preview_size *
                                parameters_.preview_size *
                                parameters_.preview_size);

    // initialize filter. TODO make choice
    filter_ = util::filter::ram_lak(geom_.cols);

    if (geom_.parallel) {
        // make reconstruction object par
        alg_ =
            std::make_unique<detail::parallel_beam_solver>(parameters_, geom_);
    } else {
        // make reconstruction object cb
        alg_ = std::make_unique<detail::cone_beam_solver>(parameters_, geom_);

        // initialize FDK weights
        fdk_weights_ = ((detail::cone_beam_solver*)(alg_.get()))->fdk_weights();
    }

    initialized_ = true;
} // namespace slicerecon

void reconstructor::transpose_sino_(std::vector<float>& projection_group,
                                    std::vector<float>& sino_buffer,
                                    int group_size) {
    // major to minor: [i, j, k]
    // In projection_group we have: [projection_id, rows, cols ]
    // For sinogram we want: [rows, projection_id, cols]

    for (int i = 0; i < geom_.rows; ++i) {
        for (int j = 0; j < group_size; ++j) {
            for (int k = 0; k < geom_.cols; ++k) {
                sino_buffer[i * group_size * geom_.cols + j * geom_.cols + k] =
                    projection_group[j * geom_.cols * geom_.rows +
                                     i * geom_.cols + k];
            }
        }
    }
}

void reconstructor::upload_(int proj_id_min, int proj_id_max) {
    if (!initialized_) {
        return;
    }

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Uploading buffer (" << write_index_
                          << ") between " << proj_id_min << "/" << proj_id_max
                          << slicerecon::util::end_log;

    environment_.spawn(parameters_.filter_cores, [&](auto& world) {
        util::process_projection(world, geom_.rows, geom_.cols,
                                 buffer_[write_index_].data(), dark_.data(),
                                 flat_fielder_.data(), filter_, proj_id_min,
                                 proj_id_max, !geom_.parallel, fdk_weights_,
                                 !parameters_.already_linear);
    });

    transpose_sino_(buffer_[write_index_], sino_buffer_,
                    proj_id_max - proj_id_min + 1);

    astra::uploadMultipleProjections(alg_->proj_data(write_index_),
                                     sino_buffer_.data(), proj_id_min,
                                     proj_id_max);

    // send message to observers that new data is available
    for (auto l : listeners_) {
        l->notify(*this);
    }
}

void reconstructor::refresh_data_() {
    if (!initialized_) {
        return;
    }

    alg_->reconstruct_preview(small_volume_buffer_, 1 - write_index_);

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Reconstructed low-res preview ("
                          << 1 - write_index_ << ")"
                          << slicerecon::util::end_log;

    // send message to observers that new data is available
    for (auto l : listeners_) {
        l->notify(*this);
    }
}

} // namespace slicerecon
