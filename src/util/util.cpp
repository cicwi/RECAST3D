#include "slicerecon/util/util.hpp"

namespace slicerecon::util {

std::vector<astra::SPar3DProjection>
list_to_par_projections(const std::vector<float>& vectors) {
    int proj_count = vectors.size() / 12;
    auto result = std::vector<astra::SPar3DProjection>(proj_count);
    for (int o = 0; o < proj_count; ++o) {
        result[o] = {
            vectors[o * 12 + 0], vectors[o * 12 + 1],  vectors[o * 12 + 2],
            vectors[o * 12 + 3], vectors[o * 12 + 4],  vectors[o * 12 + 5],
            vectors[o * 12 + 6], vectors[o * 12 + 7],  vectors[o * 12 + 8],
            vectors[o * 12 + 9], vectors[o * 12 + 10], vectors[o * 12 + 11]};
    }
    return result;
}

std::vector<astra::SConeProjection>
list_to_cone_projections(const std::vector<float>& vectors) {
    int proj_count = vectors.size() / 12;
    auto result = std::vector<astra::SConeProjection>(proj_count);
    for (int o = 0; o < proj_count; ++o) {
        result[o] = {
            vectors[o * 12 + 0], vectors[o * 12 + 1],  vectors[o * 12 + 2],
            vectors[o * 12 + 3], vectors[o * 12 + 4],  vectors[o * 12 + 5],
            vectors[o * 12 + 6], vectors[o * 12 + 7],  vectors[o * 12 + 8],
            vectors[o * 12 + 9], vectors[o * 12 + 10], vectors[o * 12 + 11]};
    }
    return result;
}

std::unique_ptr<astra::CParallelVecProjectionGeometry3D>
proj_to_vec(astra::CParallelProjectionGeometry3D* parallel_geom) {
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

std::unique_ptr<astra::CConeVecProjectionGeometry3D>
proj_to_vec(astra::CConeProjectionGeometry3D* cone_geom) {
    auto projs = cone_geom->getProjectionCount();
    auto angles = cone_geom->getProjectionAngles();
    auto rows = cone_geom->getDetectorRowCount();
    auto cols = cone_geom->getDetectorColCount();
    auto dx = cone_geom->getDetectorSpacingX();
    auto dy = cone_geom->getDetectorSpacingY();
    auto dod = cone_geom->getOriginDetectorDistance();
    auto dos = cone_geom->getOriginSourceDistance();

    auto vectors = std::vector<astra::SConeProjection>(projs);
    auto i = 0;
    for (auto& v : vectors) {
        auto vrd = Eigen::Vector3f{std::sin(angles[i]) * dos,
                                   -std::cos(angles[i]) * dos, 0.0f};
        auto vc = Eigen::Vector3f{-std::sin(angles[i]) * dod,
                                  std::cos(angles[i]) * dod, 0.0f};
        auto vdx = Eigen::Vector3f{std::cos(angles[i]) * dx,
                                   std::sin(angles[i]) * dx, 0};
        auto vdy = Eigen::Vector3f{0, 0, dy};
        vc -= 0.5f * ((float)rows * vdy + (float)cols * vdx);

        // ( srcX, srcY, srcZ, dX, dY, dZ, uX, uY, uZ, vX, vY, vZ )
        v = {// source
             vrd[0], vrd[1], vrd[2],
             // center
             vc[0], vc[1], vc[2],
             // pixel x dir
             vdx[0], vdx[1], vdx[2],
             // pixel y dir
             vdy[0], vdy[1], vdy[2]};
        ++i;
    }

    return std::make_unique<astra::CConeVecProjectionGeometry3D>(
        projs, rows, cols, vectors.data());
}

std::tuple<Eigen::Vector3f, Eigen::Matrix3f, Eigen::Vector3f>
slice_transform(Eigen::Vector3f base, Eigen::Vector3f axis_1,
                Eigen::Vector3f axis_2, float k) {
    auto rotation_onto = [](Eigen::Vector3f x,
                            Eigen::Vector3f y) -> Eigen::Matrix3f {
        auto z = x.normalized();
        auto w = y.normalized();
        auto axis = z.cross(w);
        if (axis.norm() < 0.0001f) {
            return Eigen::Matrix3f::Identity();
        }
        auto alpha = z.dot(w);
        auto angle = std::acos(alpha);
        return Eigen::AngleAxis<float>(angle, axis.normalized()).matrix();
    };

    base = base * k;
    axis_1 = axis_1 * k;
    axis_2 = axis_2 * k;
    auto delta = base + 0.5f * (axis_1 + axis_2);

    auto rot = rotation_onto(axis_1, {2.0f * k, 0.0f, 0.0f});
    rot = rotation_onto(rot * axis_2, {0.0f, 2.0f * k, 0.0f}) * rot;

    return {-delta, rot, {1.0f, 1.0f, 1.0f}};
}

} // namespace slicerecon::util
