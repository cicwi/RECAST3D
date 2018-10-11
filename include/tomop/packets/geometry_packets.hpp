#pragma once

#include <cstdint>

#include "../packets.hpp"

namespace tomop {

class GeometrySpecificationPacket
    : public PacketBase<GeometrySpecificationPacket> {
  public:
    GeometrySpecificationPacket()
        : PacketBase<GeometrySpecificationPacket>(
              packet_desc::geometry_specification),
          scene_id(-1), parallel(false),
          projections(0), volume_min_point{0.0f, 0.0f, 0.0f}, volume_max_point{
                                                                  1.0f, 1.0f,
                                                                  1.0f} {}

    GeometrySpecificationPacket(int32_t scene_id_, bool parallel_,
                                int32_t projections_)
        : PacketBase<GeometrySpecificationPacket>(
              packet_desc::geometry_specification),
          scene_id(scene_id_), parallel(parallel_),
          projections(projections_), volume_min_point{0.0f, 0.0f, 0.0f},
          volume_max_point{1.0f, 1.0f, 1.0f} {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | parallel;
        buffer | projections;
        buffer | volume_min_point;
        buffer | volume_max_point;
    }

    int32_t scene_id;
    bool parallel;
    int32_t projections;
    std::array<float, 3> volume_min_point;
    std::array<float, 3> volume_max_point;
};

class AcquisitionGeometryPacket : public PacketBase<AcquisitionGeometryPacket> {
  public:
    AcquisitionGeometryPacket()
        : PacketBase<AcquisitionGeometryPacket>(
              packet_desc::acquisition_geometry) {}

    AcquisitionGeometryPacket(int32_t scene_id_, int32_t rows_, int32_t cols_,
                              int32_t proj_count_, bool parallel_,
                              float source_origin_, float origin_det_,
                              std::vector<float> angles_)
        : PacketBase<AcquisitionGeometryPacket>(
              packet_desc::acquisition_geometry),
          scene_id(scene_id_), rows(rows_), cols(cols_),
          proj_count(proj_count_), parallel(parallel_),
          source_origin(source_origin_), origin_det(origin_det_),
          angles(angles_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | rows;
        buffer | cols;
        buffer | proj_count;
        buffer | parallel;
        buffer | source_origin;
        buffer | origin_det;
        buffer | angles;
    }

    int32_t scene_id;
    int32_t rows;
    int32_t cols;
    int32_t proj_count;
    bool parallel;
    float source_origin;
    float origin_det;
    std::vector<float> angles;
};

class ProjectionDataPacket : public PacketBase<ProjectionDataPacket> {
  public:
    ProjectionDataPacket()
        : PacketBase<ProjectionDataPacket>(packet_desc::projection_data),
          scene_id(-1), projection_id(-1) {}

    ProjectionDataPacket(int32_t scene_id_, int32_t projection_id_,
                         std::array<float, 3> source_position_,
                         std::array<float, 9> detector_orientation_,
                         std::array<int32_t, 2> detector_pixels_,
                         std::vector<float> data_)
        : PacketBase<ProjectionDataPacket>(packet_desc::projection_data),
          scene_id(scene_id_), projection_id(projection_id_),
          source_position(source_position_),
          detector_orientation(detector_orientation_),
          detector_pixels(detector_pixels_), data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | projection_id;
        buffer | source_position;
        buffer | detector_orientation;
        buffer | detector_pixels;
        buffer | data;
    }

    int32_t scene_id;
    int32_t projection_id;
    std::array<float, 3> source_position;
    std::array<float, 9> detector_orientation;
    std::array<int32_t, 2> detector_pixels;
    std::vector<float> data;
};

class PartialProjectionDataPacket
    : public PacketBase<PartialProjectionDataPacket> {
  public:
    PartialProjectionDataPacket()
        : PacketBase<PartialProjectionDataPacket>(
              packet_desc::partial_projection_data),
          scene_id(-1), projection_id(-1) {}

    PartialProjectionDataPacket(int32_t scene_id_, int32_t projection_id_,
                                std::array<float, 3> source_position_,
                                std::array<float, 9> detector_orientation_,
                                std::array<int32_t, 2> detector_pixels_,
                                std::array<int32_t, 2> partial_offset_,
                                std::array<int32_t, 2> partial_size_,
                                std::vector<float> data_)
        : PacketBase<PartialProjectionDataPacket>(
              packet_desc::partial_projection_data),
          scene_id(scene_id_), projection_id(projection_id_),
          source_position(source_position_),
          detector_orientation(detector_orientation_),
          detector_pixels(detector_pixels_), partial_offset(partial_offset_),
          partial_size(partial_size_), data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | scene_id;
        buffer | projection_id;
        buffer | source_position;
        buffer | detector_orientation;
        buffer | detector_pixels;
        buffer | partial_offset;
        buffer | partial_size;
        buffer | data;
    }

    int32_t scene_id;
    int32_t projection_id;
    std::array<float, 3> source_position;
    std::array<float, 9> detector_orientation;
    std::array<int32_t, 2> detector_pixels;
    std::array<int32_t, 2> partial_offset;
    std::array<int32_t, 2> partial_size;
    std::vector<float> data;
};

class ProjectionPacket : public PacketBase<ProjectionPacket> {
  public:
    ProjectionPacket()
        : PacketBase<ProjectionPacket>(packet_desc::projection),
          projection_id(-1) {}

    ProjectionPacket(int32_t type_, int32_t projection_id_,
                     std::array<int32_t, 2> shape_, std::vector<float> data_)
        : PacketBase<ProjectionPacket>(packet_desc::projection), type(type_),
          projection_id(projection_id_), shape(shape_), data(data_) {}

    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer | type;
        buffer | projection_id;
        buffer | shape;
        buffer | data;
    }

    // NOTE: some dependencies rely on this fixed order,
    // options are: 0 dark, 1 flat, 2 standard
    int32_t type;
    // this is cyclic: scan_id * angles + angle_idx
    int32_t projection_id;
    std::array<int32_t, 2> shape;
    std::vector<float> data;
};

} // namespace tomop
