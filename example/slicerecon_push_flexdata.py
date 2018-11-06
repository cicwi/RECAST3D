import tomop
import flexdata as flex
import numpy as np
import scipy.misc
import astra
import argparse

parser = argparse.ArgumentParser(description='Push a FleX-ray data set to Slicerecon.')
parser.add_argument('path', metavar='path',
                    help='path to the data')
parser.add_argument('--sample', type=int, default=1,
                    help='the binning to use on the detector, and how many projections to skip')
parser.add_argument('--host', default="localhost",
                    help='the projection server host')
parser.add_argument('--port', type=int, default=5558,
                    help='the projection server port')

parser.add_argument('--skipgeometry', action='store_true',
                    help='assume the geometry packet is already sent')
args = parser.parse_args()

sample = args.sample
print("sample", sample)
path = args.path

dark = flex.io.read_tiffs(path, 'di', sample=sample)
flat = flex.io.read_tiffs(path, 'io', sample=sample)
proj = flex.io.read_tiffs(path, 'scan_', sample=sample, skip=sample)

print(np.shape(dark), np.shape(flat), np.shape(proj))

avg_flat = flat.mean(0)
avg_dark = dark.mean(0)
meta = flex.io.read_meta(path, 'flexray', sample=sample)

pub = tomop.publisher(args.host, args.port)

# send astra geometry
# proj is [proj_id, row, col], we want [row, proj_id, col]
ps = np.shape(proj)
rows = ps[1]
proj_count = ps[0]
cols = ps[2]

proj_geom = flex.io.astra_proj_geom(meta['geometry'], [ps[1], ps[0], ps[2]])
# y, x, z
vol_geom = flex.io.astra_vol_geom(meta['geometry'], [rows, cols, cols])

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
if not args.skipgeometry:
    pub.send(packet_vol_geom)

packet_geometry = tomop.cone_vec_packet(0, rows, cols, proj_count,
                                        proj_geom['Vectors'].flatten())

if not args.skipgeometry:
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
