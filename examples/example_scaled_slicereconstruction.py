#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Apr  3 10:10:18 2020

@author: lagerwer
"""
import slicerecon
import astra
import numpy as np
import matplotlib.pyplot as plt
slicerecon.mute()

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


# Explicitely define a Ram--Lak filter
ram_lak = np.zeros(u)  
# mid = (u + 1) // 2
# f(0) = 1/4
# f(2k+1) = -1 / (2 k pi)^2
# f(2k) = 0

rl_spat = np.zeros(u)
mid = u // 2
rl_spat[mid] = 1 / 4
for i in range(0, mid // 2):
    rl_spat[mid - (2 * i + 1)] = -1 / ((2 * i + 1) * np.pi) ** 2
    rl_spat[mid + (2 * i + 1)] = -1 / ((2 * i + 1) * np.pi) ** 2

# Correct for the non normalized fft slicerecon does
ram_lak = np.fft.fft(np.fft.fftshift((rl_spat))) / u

# Initialize the solver
slice_size = vox
solver = slicerecon.get_solver(proj_geom, 
                               vol_geom,
                               slice_size,
                               proj_data,
                               use_custom_filter=ram_lak,
                               preview_size=1)

# weighting of the angular integral
ang_w = np.pi / (2 * ang)  

# Weighting of the convolution scale the detector pixels to the voxel size pixels
conv_w = (src_rad + det_rad) / (src_rad * usize / voxsize)

# scaling due to astra cone bp
BP_comp = usize * vsize / voxsize ** 3

# Combine the coefficients and last coefficients 
FDK_weighting = ang_w * conv_w * BP_comp / voxsize * magn ** 2 * 2


# %%
# Reconstruct the central XY slice
data = slicerecon.reconstruct_slice(solver,
                                    base=[-volsize / 2, -volsize // 2, 0],
                                    a_axis=[volsize, 0, 0],
                                    b_axis=[0, volsize, 0]) * FDK_weighting

plt.close('all')
plt.figure()
plt.imshow(data)
plt.colorbar()
plt.show()
