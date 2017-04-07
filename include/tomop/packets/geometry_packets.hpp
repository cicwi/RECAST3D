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
          scene_id(-1),
          parallel(false),
          projections(0) {}

    GeometrySpecificationPacket(int32_t scene_id_, bool parallel_, int32_t projections_)
        : PacketBase<GeometrySpecificationPacket>(
              packet_desc::geometry_specification),
          scene_id(scene_id_),
          parallel(parallel_),
          projections(projections_) {}

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

class ProjectionDataPacket : public PacketBase<ProjectionDataPacket> {
   public:
    ProjectionDataPacket()
        : PacketBase<ProjectionDataPacket>(packet_desc::projection_data),
          scene_id(-1),
          projection_id(-1) {}

    ProjectionDataPacket(int32_t scene_id_, int32_t projection_id_,
                         std::array<float, 3> source_position_,
                         std::array<float, 9> detector_orientation_,
                         std::array<int32_t, 2> detector_pixels_,
                         std::vector<uint8_t> data_)
        : PacketBase<ProjectionDataPacket>(packet_desc::projection_data),
          scene_id(scene_id_),
          projection_id(projection_id_),
          source_position(source_position_),
          detector_orientation(detector_orientation_),
          detector_pixels(detector_pixels_),
          data(data_) {}

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
    std::vector<uint8_t> data;
};

}  // namespace tomop
