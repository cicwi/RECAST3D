import tomop
import flexbox as flex
import numpy as np
import scipy.misc
import astra

import sys

if len(sys.argv) < 2:
    print("ERROR: No path given")
    exit(-1)

path = sys.argv[1]

dark = flex.data.read_raw(path, 'di', sample=[2, 2])
flat = flex.data.read_raw(path, 'io', sample=[2, 2])
proj = flex.data.read_raw(path, 'scan_', sample=[2, 2], skip=2)

print(np.shape(dark), np.shape(flat), np.shape(proj))

avg_flat = flat.mean(0)
avg_dark = flex.data.read_raw(path, 'di', sample=[2, 2])

meta = flex.data.read_log(path, 'flexray', bins=2)

pub = tomop.publisher("localhost", 5558)

# send astra geometry
# proj is [proj_id, row, col], we want [row, proj_id, col]
ps = np.shape(proj)
rows = ps[1]
proj_count = ps[0]
cols = ps[2]

proj_geom = flex.data.astra_proj_geom(meta['geometry'], [ps[1], ps[0], ps[2]])
# y, x, z
vol_geom = flex.data.astra_vol_geom(meta['geometry'], [rows, cols, cols])

print(proj_geom)
print('proj geom vector count', len(proj_geom['Vectors']))
print(vol_geom)

packet_vol_geom = tomop.geometry_specification_packet(0, [
    vol_geom['option']['WindowMinX'], vol_geom['option']['WindowMinY'],
    vol_geom['option']['WindowMinZ']
], [
    vol_geom['option']['WindowMaxX'], vol_geom['option']['WindowMaxY'],
    vol_geom['option']['WindowMaxZ']
])
pub.send(packet_vol_geom)

packet_geometry = tomop.cone_vec_packet(0, rows, cols, proj_count,
                                        proj_geom['Vectors'].flatten())

pub.send(packet_geometry)

# send darks (0), lights (1), projs (2)

packet_dark = tomop.projection_packet(0, 0, [rows, cols],
                                      np.ascontiguousarray(avg_dark.flatten()))
pub.send(packet_dark)

packet_light = tomop.projection_packet(
    1, 0, [rows, cols], np.ascontiguousarray(avg_flat.flatten()))
pub.send(packet_light)

for i in np.arange(0, proj_count):
    packet_proj = tomop.projection_packet(
        2, i, [rows, cols], np.ascontiguousarray(proj[i].flatten()))
    pub.send(packet_proj)
