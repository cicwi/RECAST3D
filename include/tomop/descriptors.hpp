#pragma once

namespace tomop {

enum class packet_desc : int {
    // SCENE MANAGEMENT
    make_scene,

    // RECONSTRUCTION
    slice_data,
    volume_data,
    set_slice,
    remove_slice,
    group_request_slices,

    // GEOMETRY
    geometry_specification,
    projection_data,

    // PARTITIONING
    set_part,
};

}  // namespace tomop
