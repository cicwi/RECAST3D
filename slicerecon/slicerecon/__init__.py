import astra
import numpy as np

from py_slicerecon import *

__all__ = [
    "plugin", "acquisition_geometry", "reconstructor", "settings",
    "paganin_settings", "get_solver", "reconstruct_slice", "mute"
]

def convert_acquisition_geometry(proj_geom, vol_geom):
    geom_type = proj_geom['type']

    if geom_type not in ['cone', 'cone_vec', 'parallel3d', 'parallel3d_vec']:
        raise Exception("Only 3D cone-beam or parallel-beam geometries supported" )

    if geom_type == 'cone' or geom_type == 'parallel3d':
       proj_geom =  astra.functions.geom_2vec(proj_geom)

    rows = proj_geom['DetectorRowCount']
    cols = proj_geom['DetectorColCount']
    parallel = geom_type == 'parallel3d' or geom_type == 'parallel3d_vec'
    vec_geometry = True
    angles = np.array(proj_geom['Vectors'], dtype=np.float32).ravel()
    print(angles[0:12])
    proj_count = len(angles) // 12
    print("proj_count", proj_count)

    opts = vol_geom['option']
    volume_min_point = [opts['WindowMinX'], opts['WindowMinY'], opts['WindowMinZ']]
    volume_max_point = [opts['WindowMaxX'], opts['WindowMaxY'], opts['WindowMaxZ']]

    return acquisition_geometry(rows=rows, cols=cols, proj_count=proj_count, angles=angles, parallel=parallel, vec_geometry=vec_geometry, volume_min_point=volume_min_point, volume_max_point=volume_max_point)


def get_solver(proj_geom, vol_geom, slice_size, proj_data, use_custom_filter=None, **kwargs):
    parameters = settings(slice_size=slice_size, **kwargs)
    solver = reconstructor(parameters)

    acq_geom = convert_acquisition_geometry(proj_geom, vol_geom)

    solver.initialize(acq_geom)

    if use_custom_filter is not None:
        solver.set_filter(use_custom_filter)

    proj_data = np.transpose(proj_data, [1, 0, 2])

    for idx in range(proj_data.shape[0]):
        push_projection(solver, idx, np.ascontiguousarray(proj_data[idx, :, :]))

    return solver


def reconstruct_slice(solver, base, a_axis, b_axis):
    shape, data = solver.reconstruct_slice(a_axis + b_axis + base)
    return np.array(data).reshape(shape)
