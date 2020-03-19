import tomop
from flexdata import data
import numpy as np
import scipy.misc
import astra
import argparse

parser = argparse.ArgumentParser(
    description='Push a FleX-ray data set to Slicerecon.')
parser.add_argument('path', metavar='path', help='path to the data')
parser.add_argument(
    '--sample',
    type=int,
    default=1,
    help='the binning to use on the detector, and how many projections to skip'
)
parser.add_argument(
    '--host', default="localhost", help='the projection server host')
parser.add_argument(
    '--port', type=int, default=5558, help='the projection server port')

parser.add_argument(
    '--skipgeometry',
    action='store_true',
    help='assume the geometry packet is already sent')
args = parser.parse_args()

sample = args.sample
print("sample", sample)
path = args.path

dark = data.read_stack(path, 'di00', sample = sample)
flat = data.read_stack(path, 'io00', sample = sample)    
proj = data.read_stack(path, 'scan_', skip = sample, sample = sample)

print(np.shape(dark), np.shape(flat), np.shape(proj))

# avg_flat = flat.mean(0)
# avg_dark = dark.mean(0)
geom = data.read_flexraylog(path, sample=sample)

pub = tomop.publisher(args.host, args.port)

# send astra geometry
# proj is [proj_id, row, col], we want [row, proj_id, col]
ps = np.shape(proj)
rows = ps[0]
proj_count = ps[1]
cols = ps[2]

vol_geom = geom.astra_volume_geom([rows, cols, cols])
proj_geom = geom.astra_projection_geom(proj.shape)

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

print("flat dark", flat.shape, dark.shape)
packet_scan_settings = tomop.scan_settings_packet(0, dark.shape[1],
                                                  flat.shape[1], False)
if not args.skipgeometry:
    pub.send(packet_scan_settings)

packet_geometry = tomop.cone_vec_geometry_packet(
    0, rows, cols, proj_count, proj_geom['Vectors'].flatten())

if not args.skipgeometry:
    pub.send(packet_geometry)

# send darks (0), lights (1), projs (2)

for i in np.arange(0, dark.shape[1]):
    packet_dark = tomop.projection_packet(
        0, i, [rows, cols], np.ascontiguousarray(dark[:, i, :].flatten()))
    pub.send(packet_dark)

for i in np.arange(0, flat.shape[1]):
    packet_light = tomop.projection_packet(
        1, i, [rows, cols], np.ascontiguousarray(flat[:, i, :].flatten()))
    pub.send(packet_light)

for i in np.arange(0, proj_count):
    packet_proj = tomop.projection_packet(
            2, i, [rows, cols], np.ascontiguousarray(proj[:, i, :].flatten()))
    pub.send(packet_proj)
