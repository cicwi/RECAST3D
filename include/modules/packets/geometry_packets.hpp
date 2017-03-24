#pragma once

#include "packets.hpp"

namespace tomovis {

class GeometrySpecificationPacket
    : public PacketBase<GeometrySpecificationPacket> {
   public:
    GeometrySpecificationPacket()
        : PacketBase<GeometrySpecificationPacket>(
              packet_desc::geometry_specification),
          scene_id(-1),
          parallel(false),
          projections(0) {}

    GeometrySpecificationPacket(int scene_id_, bool parallel_, int projections_)
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

    int scene_id;
    bool parallel;
    int projections;
    std::array<float, 3> volume_min_point;
    std::array<float, 3> volume_max_point;
};

class ProjectionDataPacket : public PacketBase<ProjectionDataPacket> {
   public:
    ProjectionDataPacket()
        : PacketBase<ProjectionDataPacket>(packet_desc::projection_data),
          scene_id(-1),
          projection_id(-1) {}

    ProjectionDataPacket(int scene_id_, int projection_id_,
                         std::array<float, 3> source_position_,
                         std::array<float, 9> detector_orientation_,
                         std::array<int, 2> detector_pixels_,
                         std::vector<unsigned char> data_)
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

    int scene_id;
    int projection_id;
    std::array<float, 3> source_position;
    std::array<float, 9> detector_orientation;
    std::array<int, 2> detector_pixels;
    std::vector<unsigned char> data;
};

}  // namespace tomovis
