#pragma once

namespace tomop {

enum class packet_desc : int {
    // SCENE MANAGEMENT
    make_scene,
    kill_scene,

    // RECONSTRUCTION
    slice_data,
    partial_slice_data,
    volume_data,
    partial_volume_data,
    set_slice,
    remove_slice,
    group_request_slices,
    post_process_set,

    // GEOMETRY
    geometry_specification,
    projection_data,
    partial_projection_data,
    projection,

    // PARTITIONING
    set_part,
};

}  // namespace tomop
