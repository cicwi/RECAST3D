#include "slicerecon/reconstruction/reconstructor.hpp"
#include "slicerecon/util/processing.hpp"
#include "slicerecon/util/util.hpp"

namespace slicerecon {

namespace detail {

solver::solver(settings parameters, acquisition::geometry geometry)
    : parameters_(parameters), geometry_(geometry) {

    // 1a) convert to par vec

    // 3) TODO: MOVE UP create volume geometry
    vol_geom_ = std::make_unique<astra::CVolumeGeometry3D>(
        parameters_.slice_size, parameters_.slice_size, 1);
    // 4) TODO: MOVE UP create volume data
    vol_handle_ = astraCUDA3d::allocateGPUMemory(parameters_.slice_size,
                                                 parameters_.slice_size, 1,
                                                 astraCUDA3d::INIT_ZERO);
    vol_data_ = std::make_unique<astra::CFloat32VolumeData3DGPU>(
        vol_geom_.get(), vol_handle_);

    // TODO: MOVE UP SMALL PREVIEW VOL
    vol_geom_small_ = std::make_unique<astra::CVolumeGeometry3D>(
        parameters_.preview_size, parameters_.preview_size,
        parameters_.preview_size, vol_geom_->getWindowMinX(),
        vol_geom_->getWindowMinX(), vol_geom_->getWindowMinX(),
        vol_geom_->getWindowMaxX(), vol_geom_->getWindowMaxX(),
        vol_geom_->getWindowMaxX());
    vol_handle_small_ = astraCUDA3d::allocateGPUMemory(
        parameters_.preview_size, parameters_.preview_size,
        parameters_.preview_size, astraCUDA3d::INIT_ZERO);
    vol_data_small_ = std::make_unique<astra::CFloat32VolumeData3DGPU>(
        vol_geom_small_.get(), vol_handle_small_);
}

parallel_beam_solver::parallel_beam_solver(settings parameters,
                                           acquisition::geometry geometry)
    : solver(parameters, geometry) {
    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Initializing parallel beam solver"
                          << slicerecon::util::end_log;

    // 1) create projection geometry
    auto proj_geom = astra::CParallelProjectionGeometry3D(
        geometry_.proj_count, geometry_.rows, geometry_.cols, 1.0f, 1.0f,
        geometry_.angles.data());

    proj_geom_ = slicerecon::util::proj_to_vec(&proj_geom);
    proj_geom_small_ = slicerecon::util::proj_to_vec(&proj_geom);
    vectors_ = std::vector<astra::SPar3DProjection>(
        proj_geom_->getProjectionVectors(),
        proj_geom_->getProjectionVectors() + geometry_.proj_count);
    vec_buf_ = vectors_;

    auto zeros = std::vector<float>(
        geometry_.proj_count * geometry_.cols * geometry_.rows, 0.0f);

    // 2) create projection data..
    for (int i = 0; i < 2; ++i) {
        proj_handles_.push_back(astraCUDA3d::createProjectionArrayHandle(
            zeros.data(), geometry_.cols, geometry_.proj_count,
            geometry_.rows));
        proj_datas_.push_back(
            std::make_unique<astra::CFloat32ProjectionData3DGPU>(
                proj_geom_.get(), proj_handles_[0]));
    }
    // 5) create back projection algorithm, link to previously
    // made objects
    projector_ = std::make_unique<astra::CCudaProjector3D>();
    // make algorithm for two targets if mode is single
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
    // TODO
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
    filter_ = util::filter::shepp_logan(geom_.cols);

    if (geom_.parallel) {
        // make recosntruction object par
        alg_ =
            std::make_unique<detail::parallel_beam_solver>(parameters_, geom_);
    } else {
        // make reconstruction object cb
        alg_ = std::make_unique<detail::cone_beam_solver>(parameters_, geom_);
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
                                 flat_fielder_.data(), filter_);
    });

    transpose_sino_(buffer_[write_index_], sino_buffer_,
                    proj_id_max - proj_id_min + 1);

    astra::uploadMultipleProjections(alg_->proj_data(write_index_),
                                     sino_buffer_.data(), proj_id_min,
                                     proj_id_max, false);
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
