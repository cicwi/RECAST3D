#include "slicerecon/reconstruction/helpers.hpp"

namespace slicerecon {

std::unique_ptr<astra::CParallelVecProjectionGeometry3D>
parallel_beam_to_vec(astra::CParallelProjectionGeometry3D* parallel_geom) {
    auto projs = parallel_geom->getProjectionCount();
    auto angles = parallel_geom->getProjectionAngles();
    auto rows = parallel_geom->getDetectorRowCount();
    auto cols = parallel_geom->getDetectorColCount();
    auto dx = parallel_geom->getDetectorSpacingX();
    auto dy = parallel_geom->getDetectorSpacingY();

    auto vectors = std::vector<astra::SPar3DProjection>(projs);
    auto i = 0;
    for (auto& v : vectors) {
        auto vrd =
            Eigen::Vector3f{std::sin(angles[i]), -std::cos(angles[i]), 0};
        auto vc = Eigen::Vector3f{0, 0, 0};
        auto vdx = Eigen::Vector3f{std::cos(angles[i]) * dx,
                                   std::sin(angles[i]) * dx, 0};
        auto vdy = Eigen::Vector3f{0, 0, dy};
        vc -= 0.5f * ((float)rows * vdy + (float)cols * vdx);
        v = {// ray dir
             vrd[0], vrd[1], vrd[2],
             // center
             vc[0], vc[1], vc[2],
             // pixel x dir
             vdx[0], vdx[1], vdx[2],
             // pixel y dir
             vdy[0], vdy[1], vdy[2]};
        ++i;
    }

    return std::make_unique<astra::CParallelVecProjectionGeometry3D>(
        projs, rows, cols, vectors.data());
}

} // namespace slicerecon
