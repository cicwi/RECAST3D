import slicerecon
import astra
import numpy as np
import matplotlib.pyplot as plt


angles = np.linspace(0, 2.0 * np.pi, 360, False)
proj_geom = astra.create_proj_geom('cone', 2.0, 2.0, 128, 128, angles, 512.0, 512.0)
vol_geom = astra.create_vol_geom(128, 128, 128)

cube = np.zeros((128,128,128))
cube[17:113,17:113,17:113] = 1
cube[33:97,33:97,33:97] = 0
proj_id, proj_data = astra.create_sino3d_gpu(cube, proj_geom, vol_geom)

slice_size = 512
solver = slicerecon.get_solver(proj_geom, vol_geom, slice_size, proj_data)

data = slicerecon.reconstruct_slice(solver, base=[-64, -64, 0], a_axis=[128, 0, 0], b_axis=[0, 128, 0])
plt.imshow(data)
plt.show()

preview = np.array(solver.preview()).reshape(cube.shape)
plt.imshow(preview[64, :, :])
plt.show()
