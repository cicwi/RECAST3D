#include <sstream>

#include "slicerecon/util/util.hpp"

namespace slicerecon::util {

std::vector<astra::SPar3DProjection>
list_to_par_projections(int rows, int cols, const std::vector<float>& vectors) {
    int proj_count = vectors.size() / 12;
    auto result = std::vector<astra::SPar3DProjection>(proj_count);
    for (int o = 0; o < proj_count; ++o) {
        auto ray = Eigen::Vector3f{vectors[o * 12 + 0], vectors[o * 12 + 1],
                                   vectors[o * 12 + 2]};
        auto det = Eigen::Vector3f{vectors[o * 12 + 3], vectors[o * 12 + 4],
                                   vectors[o * 12 + 5]};
        auto vx = Eigen::Vector3f{vectors[o * 12 + 6], vectors[o * 12 + 7],
                                  vectors[o * 12 + 8]};
        auto vy = Eigen::Vector3f{vectors[o * 12 + 9], vectors[o * 12 + 10],
                                  vectors[o * 12 + 11]};

        det -= 0.5f * (rows * vy + cols * vx);

        result[o] = {ray[0], ray[1], ray[2], det[0], det[1], det[2],
                     vx[0],  vx[1],  vx[2],  vy[0],  vy[1],  vy[2]};
    }
    return result;
}

std::vector<astra::SConeProjection>
list_to_cone_projections(int rows, int cols, const std::vector<float>& vectors) {
    int proj_count = vectors.size() / 12;
    auto result = std::vector<astra::SConeProjection>(proj_count);
    for (int o = 0; o < proj_count; ++o) {
        auto src = Eigen::Vector3f{vectors[o * 12 + 0], vectors[o * 12 + 1],
                                   vectors[o * 12 + 2]};
        auto det = Eigen::Vector3f{vectors[o * 12 + 3], vectors[o * 12 + 4],
                                   vectors[o * 12 + 5]};
        auto vx = Eigen::Vector3f{vectors[o * 12 + 6], vectors[o * 12 + 7],
                                  vectors[o * 12 + 8]};
        auto vy = Eigen::Vector3f{vectors[o * 12 + 9], vectors[o * 12 + 10],
                                  vectors[o * 12 + 11]};

        det -= 0.5f * (rows * vy + cols * vx);

        result[o] = {src[0], src[1], src[2], det[0], det[1], det[2],
                     vx[0],  vx[1],  vx[2],  vy[0],  vy[1],  vy[2]};
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
        auto vrd = Eigen::Vector3f{std::sin(angles[i]), -std::cos(angles[i]), 0};
        auto vc = Eigen::Vector3f{0, 0, 0};
        auto vdx =
        Eigen::Vector3f{std::cos(angles[i]) * dx, std::sin(angles[i]) * dx, 0};
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

    return std::make_unique<astra::CParallelVecProjectionGeometry3D>(projs, rows, cols,
                                                                     vectors.data());
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
        auto vrd =
        Eigen::Vector3f{std::sin(angles[i]) * dos, -std::cos(angles[i]) * dos, 0.0f};
        auto vc =
        Eigen::Vector3f{-std::sin(angles[i]) * dod, std::cos(angles[i]) * dod, 0.0f};
        auto vdx =
        Eigen::Vector3f{std::cos(angles[i]) * dx, std::sin(angles[i]) * dx, 0};
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

    return std::make_unique<astra::CConeVecProjectionGeometry3D>(projs, rows, cols,
                                                                 vectors.data());
}

std::tuple<Eigen::Vector3f, Eigen::Matrix3f, Eigen::Vector3f>
slice_transform(Eigen::Vector3f base,
                Eigen::Vector3f axis_1,
                Eigen::Vector3f axis_2,
                Eigen::Vector3f vol_min,
                Eigen::Vector3f vol_max) {
    // transform base
    base = vol_min +
           (0.5f * (base + Eigen::Vector3f(1.0f, 1.0f, 1.0f))).cwiseProduct(vol_max - vol_min);

    // transform axis_1 and axis_2
    axis_1 = 0.5f * axis_1 * (vol_max[0] - vol_min[0]);
    axis_2 = 0.5f * axis_2 * (vol_max[0] - vol_min[0]);

    auto rotation_onto = [](Eigen::Vector3f x, Eigen::Vector3f y) -> Eigen::Matrix3f {
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

    auto delta = base + 0.5f * (axis_1 + axis_2);

    auto rot = rotation_onto(axis_1, {vol_max[0] - vol_min[0], 0.0f, 0.0f});
    rot = rotation_onto(rot * axis_2, {0.0f, vol_max[1] - vol_min[1], 0.0f}) * rot;

    return {-delta, rot, {1.0f, 1.0f, 1.0f}};
}

std::string info(const astra::CConeVecProjectionGeometry3D& x) {
    auto ss = std::stringstream("");

    auto vectors = x.getProjectionVectors();

    ss << "DetectorRowCount: " << x.getDetectorRowCount() << ", ";
    ss << "DetectorColCount: " << x.getDetectorColCount() << ", ";
    ss << "ProjectionCount: " << x.getProjectionCount() << ", ";
    ss << "Vectors: [[" << vectors[0].fSrcX << ", " << vectors[0].fSrcY << ", "
       << vectors[0].fSrcZ << " ... " << vectors[0].fDetVY << ", " << vectors[0].fDetVZ
       << "], [" << vectors[1].fSrcX << ", " << vectors[1].fSrcY << "...]...]";

    return ss.str();
}

std::string info(const astra::CParallelVecProjectionGeometry3D& x) {
    auto ss = std::stringstream("");

    auto vectors = x.getProjectionVectors();

    ss << "DetectorRowCount: " << x.getDetectorRowCount() << ", ";
    ss << "DetectorColCount: " << x.getDetectorColCount() << ", ";
    ss << "ProjectionCount: " << x.getProjectionCount() << ", ";
    ss << "Vectors: [[" << vectors[0].fRayX << ", " << vectors[0].fRayY << ", "
       << vectors[0].fRayZ << " ... " << vectors[0].fDetVY << ", " << vectors[0].fDetVZ
       << "], [" << vectors[1].fRayX << ", " << vectors[1].fRayY << "...]...]";

    return ss.str();
}

std::string info(const astra::CVolumeGeometry3D& x) {
    auto ss = std::stringstream("");

    ss << "Min: [" << x.getWindowMinX() << ", " << x.getWindowMinY() << ", "
       << x.getWindowMinZ() << "], ";
    ss << "Max: [" << x.getWindowMaxX() << ", " << x.getWindowMaxY() << ", "
       << x.getWindowMaxZ() << "], ";
    ss << "Shape: [" << x.getGridRowCount() << ", " << x.getGridColCount()
       << ", " << x.getGridSliceCount() << "]";

    return ss.str();
}

} // namespace slicerecon::util
