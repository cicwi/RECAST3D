#pragma once

#include <cstdint>

#include "../packets.hpp"

namespace tomop {

struct GeometrySpecificationPacket
    : public PacketBase<GeometrySpecificationPacket> {
    static constexpr auto desc = packet_desc::geometry_specification;
    GeometrySpecificationPacket() = default;
    GeometrySpecificationPacket(int32_t a, std::array<float, 3> b,
                                std::array<float, 3> c)
        : scene_id(a), volume_min_point(b), volume_max_point(c) {}
    BOOST_HANA_DEFINE_STRUCT(GeometrySpecificationPacket, (int32_t, scene_id),
                             (std::array<float, 3>, volume_min_point),
                             (std::array<float, 3>, volume_max_point));
};

struct ScanSettingsPacket : public PacketBase<ScanSettingsPacket> {
    static constexpr auto desc = packet_desc::scan_settings;
    ScanSettingsPacket() = default;
    ScanSettingsPacket(int32_t a, int32_t b, int32_t c, bool d)
        : scene_id(a), darks(b), flats(c), already_linear(d) {}
    BOOST_HANA_DEFINE_STRUCT(ScanSettingsPacket, (int32_t, scene_id),
                             (int32_t, darks), (int32_t, flats),
                             (bool, already_linear));
};

struct ParallelBeamGeometryPacket
    : public PacketBase<ParallelBeamGeometryPacket> {
    static constexpr auto desc = packet_desc::parallel_beam_geometry;
    ParallelBeamGeometryPacket() = default;
    ParallelBeamGeometryPacket(int32_t a, int32_t b, int32_t c, int32_t d,
                               std::vector<float> e)
        : scene_id(a), rows(b), cols(c), proj_count(d), angles(e) {}
    BOOST_HANA_DEFINE_STRUCT(ParallelBeamGeometryPacket, (int32_t, scene_id),
                             (int32_t, rows), (int32_t, cols),
                             (int32_t, proj_count),
                             (std::vector<float>, angles));
};

struct ParallelVecGeometryPacket
    : public PacketBase<ParallelVecGeometryPacket> {
    static constexpr auto desc = packet_desc::parallel_vec_geometry;
    ParallelVecGeometryPacket() = default;
    ParallelVecGeometryPacket(int32_t a, int32_t b, int32_t c, int32_t d,
                              std::vector<float> e)
        : scene_id(a), rows(b), cols(c), proj_count(d), vectors(e) {}
    BOOST_HANA_DEFINE_STRUCT(ParallelVecGeometryPacket, (int32_t, scene_id),
                             (int32_t, rows), (int32_t, cols),
                             (int32_t, proj_count),
                             (std::vector<float>, vectors));
};

struct ConeBeamGeometryPacket : public PacketBase<ConeBeamGeometryPacket> {
    static constexpr auto desc = packet_desc::cone_beam_geometry;
    ConeBeamGeometryPacket() = default;
    ConeBeamGeometryPacket(int32_t a, int32_t b, int32_t c, int32_t d, float e,
                           float f, std::array<float, 2> g,
                           std::vector<float> h)
        : scene_id(a), rows(b), cols(c), proj_count(d), source_origin(e),
          origin_det(f), detector_size(g), angles(h) {}
    BOOST_HANA_DEFINE_STRUCT(ConeBeamGeometryPacket, (int32_t, scene_id),
                             (int32_t, rows), (int32_t, cols),
                             (int32_t, proj_count), (float, source_origin),
                             (float, origin_det),
                             (std::array<float, 2>, detector_size),
                             (std::vector<float>, angles));
};

struct ConeVecGeometryPacket : public PacketBase<ConeVecGeometryPacket> {
    static constexpr auto desc = packet_desc::cone_vec_geometry;
    ConeVecGeometryPacket() = default;
    ConeVecGeometryPacket(int32_t a, int32_t b, int32_t c, int32_t d,
                          std::vector<float> e)
        : scene_id(a), rows(b), cols(c), proj_count(d), vectors(e) {}
    BOOST_HANA_DEFINE_STRUCT(ConeVecGeometryPacket, (int32_t, scene_id),
                             (int32_t, rows), (int32_t, cols),
                             (int32_t, proj_count),
                             (std::vector<float>, vectors));
};

struct ProjectionDataPacket : public PacketBase<ProjectionDataPacket> {
    static constexpr auto desc = packet_desc::projection_data;
    ProjectionDataPacket() = default;
    ProjectionDataPacket(int32_t a, int32_t b, std::array<float, 3> c,
                         std::array<float, 9> d, std::array<int32_t, 2> e,
                         std::vector<float> f)
        : scene_id(a), projection_id(b), source_position(c),
          detector_orientation(d), detector_pixels(e), data(f) {}
    BOOST_HANA_DEFINE_STRUCT(ProjectionDataPacket, (int32_t, scene_id),
                             (int32_t, projection_id),
                             (std::array<float, 3>, source_position),
                             (std::array<float, 9>, detector_orientation),
                             (std::array<int32_t, 2>, detector_pixels),
                             (std::vector<float>, data));
};

struct PartialProjectionDataPacket
    : public PacketBase<PartialProjectionDataPacket> {
    static constexpr auto desc = packet_desc::partial_projection_data;
    PartialProjectionDataPacket() = default;
    PartialProjectionDataPacket(int32_t a, int32_t b, std::array<float, 3> c,
                                std::array<float, 9> d,
                                std::array<int32_t, 2> e,
                                std::array<int32_t, 2> f,
                                std::array<int32_t, 2> g, std::vector<float> h)
        : scene_id(a), projection_id(b), source_position(c),
          detector_orientation(d), detector_pixels(e), partial_offset(f),
          partial_size(g), data(h) {}
    BOOST_HANA_DEFINE_STRUCT(PartialProjectionDataPacket, (int32_t, scene_id),
                             (int32_t, projection_id),
                             (std::array<float, 3>, source_position),
                             (std::array<float, 9>, detector_orientation),
                             (std::array<int32_t, 2>, detector_pixels),
                             (std::array<int32_t, 2>, partial_offset),
                             (std::array<int32_t, 2>, partial_size),
                             (std::vector<float>, data));
};

struct ProjectionPacket : public PacketBase<ProjectionPacket> {
    static constexpr auto desc = packet_desc::projection;
    ProjectionPacket() = default;
    ProjectionPacket(int32_t type_, int32_t projection_id_,
                     std::array<int32_t, 2> shape_, std::vector<float> data_)
        : type(type_), projection_id(projection_id_), shape(shape_),
          data(data_) {}
    BOOST_HANA_DEFINE_STRUCT(ProjectionPacket,
                             // NOTE: some dependencies rely on this fixed
                             // order, options are: 0 dark, 1 flat, 2 standard
                             (int32_t, type),
                             // this is cyclic: scan_id * angles + angle_idx
                             (int32_t, projection_id),
                             (std::array<int32_t, 2>, shape),
                             (std::vector<float>, data));
};

} // namespace tomop
