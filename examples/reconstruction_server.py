import tomop
import numpy as np

import astra
import slicerecon

# Set the source radius and the detector radius and numbre of angles
src_rad = 1000
det_rad = 1000
ang = 360

# Number of voxels of one side of the square volume 
vox = 256
volsize = vox
voxsize = volsize / vox

# Number of detector pixels, u --> collumns, v --> rows
u = 2 * vox
v = vox
angles = np.linspace(0, 2.0 * np.pi, ang, False)

# The conebeam projection introduces a magnification
magn = src_rad / (src_rad + det_rad)

# scale the detector pixels as if they are at the origin
usize = voxsize / magn 
vsize = voxsize / magn
proj_geom = astra.create_proj_geom('cone', usize, vsize, v, u,
                                   angles, src_rad, det_rad)

# Create the astra volume geometry
vol_geom = astra.create_vol_geom(vox, vox, vox, -volsize/2, volsize/2,
                                 -volsize /2, volsize/2,
                                 -volsize/2, volsize/2)

# Create a hollow cube phantom
cube = np.zeros((vox, vox, vox))
cube[2 * vox // 8:6 * vox // 8, 2 * vox // 8:6 * vox // 8,
     2 * vox // 8:6 * vox // 8] = 1
cube[3 * vox // 8:5 * vox // 8, 3 * vox // 8:5 * vox // 8,
     3 * vox // 8:5 * vox // 8] = 0

proj_id, proj_data = astra.create_sino3d_gpu(cube, proj_geom, vol_geom)

slice_size = vox
solver = slicerecon.get_solver(proj_geom,
                               vol_geom,
                               slice_size,
                               proj_data,
                               preview_size=1)

def callback(orientation, slice_id):
    base = orientation[6:9]
    a_axis = orientation[0:3]
    b_axis = orientation[3:6]
    data = slicerecon.reconstruct_slice(solver,
                                        base=base,
                                        a_axis=a_axis,
                                        b_axis=b_axis)
    return list(data.shape), data.ravel()

serv = tomop.server("Custom reconstructor")

vdp = tomop.volume_data_packet(
    serv.scene_id(),
    list(cube.shape),
    cube.ravel())

serv.send(vdp)

serv.set_callback(callback)
serv.serve()
