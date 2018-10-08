#include "slicerecon/reconstruction/reconstructor.hpp"

namespace slicerecon {

reconstructor::reconstructor(acquisition::geometry geom, settings parameters)
    : geom_(geom), parameters_(parameters) {
    // init astra
    initialize_astra_();
    pixels_ = geom_.cols * geom_.rows;
    if (geom_.parallel) {
        // make recosntruction object par
    } else {
        // make recosntruction object cb
    }
}

void reconstructor::initialize_astra_() {
//    if (geom_.parallel) {
//        auto& g = geom_;
//        // auto& o = std::get<detail::parallel_beam_objs>(objs_);
//
//        // set up the projection geometry
//        std::vector<float> angles(g.proj_count);
//        for (auto i = 0; i < g.proj_count; ++i) {
//            angles[i] = (i * M_PI) / g.proj_count;
//        }
//        auto proj_geom = astra::CParallelProjectionGeometry3D(
//            g.proj_count, g.rows, g.cols, 1.0f, 1.0f, g.angles.data());
//        o.proj_geom = parallel_beam_to_vec(&proj_geom);
//        o.proj_geom_small = parallel_beam_to_vec(&proj_geom);
//        o.vectors = std::vector<astra::SPar3DProjection>(
//            o.proj_geom->getProjectionVectors(),
//            o.proj_geom->getProjectionVectors() + g.proj_count);
//        o.vec_buf = o.vectors;
//
//        // set up the projection data
//        auto zeros = std::vector<float>(g.proj_count * g.cols * g.rows, 0.0f);
//        for (int i = 0; i < 2; ++i) {
//            o.proj_handles.push_back(astraCUDA3d::createProjectionArrayHandle(
//                zeros.data(), g.cols, g.proj_count, g.rows));
//            o.proj_datas.push_back(
//                std::make_unique<astra::CFloat32ProjectionData3DGPU>(
//                    o.proj_geom.get(), o.proj_handles[0]));
//        }
//
//        // create volume geometry
//        o.vol_geom = std::make_unique<astra::CVolumeGeometry3D>(
//            parameters_.slice_size, parameters_.slice_size, 1);
//        // create volume data
//        o.vol_handle = astraCUDA3d::allocateGPUMemory(parameters_.slice_size,
//                                                      parameters_.slice_size, 1,
//                                                      astraCUDA3d::INIT_ZERO);
//        o.vol_data = std::make_unique<astra::CFloat32VolumeData3DGPU>(
//            o.vol_geom.get(), o.vol_handle);
//
//        // create small volume preview
//        o.vol_geom_small = std::make_unique<astra::CVolumeGeometry3D>(
//            parameters_.preview_size, parameters_.preview_size,
//            parameters_.preview_size, o.vol_geom->getWindowMinX(),
//            o.vol_geom->getWindowMinX(), o.vol_geom->getWindowMinX(),
//            o.vol_geom->getWindowMaxX(), o.vol_geom->getWindowMaxX(),
//            o.vol_geom->getWindowMaxX());
//        o.vol_handle_small = astraCUDA3d::allocateGPUMemory(
//            parameters_.preview_size, parameters_.preview_size,
//            parameters_.preview_size, astraCUDA3d::INIT_ZERO);
//        o.vol_data_small = std::make_unique<astra::CFloat32VolumeData3DGPU>(
//            o.vol_geom_small.get(), o.vol_handle_small);
//
//        o.projector = std::make_unique<astra::CCudaProjector3D>();
//        for (int i = 0; i < 2; ++i) {
//            o.algs.push_back(
//                std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
//                    o.projector.get(), o.proj_datas[i].get(),
//                    o.vol_data.get()));
//            o.algs_small.push_back(
//                std::make_unique<astra::CCudaBackProjectionAlgorithm3D>(
//                    o.projector.get(), o.proj_datas[i].get(),
//                    o.vol_data_small.get()));
//        }
//
//        // create filter (TODO separate functions)
//        // TODO migrate this somewhere else and add algebraic filters
//        // 1) ram-lak
//        auto mid = (g.cols + 1) / 2;
//        filter_.resize(g.cols);
//        for (int i = 0; i < mid; ++i) {
//            filter_[i] = i;
//        }
//        for (int j = mid; j < g.cols; ++j) {
//            filter_[j] = 2 * mid - j;
//        }
//        // 2) something more advanced
//        auto filter_weight = [=](auto i) {
//            return std::sin(M_PI * (i / (float)mid)) /
//                   (M_PI * (i / (float)mid));
//        };
//        for (int i = 1; i < mid; ++i) {
//            filter_[i] *= filter_weight(i);
//        }
//        for (int j = mid; j < g.cols; ++j) {
//            filter_[j] = filter_weight(2 * mid - j);
//        }
//    } else if (std::holds_alternative<acquisition::circular_cone_beam>(geom_)) {
//        std::cerr << "`initialize_astra_`: CCB not yet implemented...\n";
//        exit(-1);
//    }
}

} // namespace slicerecon
