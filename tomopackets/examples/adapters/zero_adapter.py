import tomop
import numpy as np
import argparse

parser = argparse.ArgumentParser(
    description='Push a data set consisting of zeros to Slicerecon.')

parser.add_argument('--host',
                    default="localhost",
                    help='the projection server host')
parser.add_argument('--port',
                    type=int,
                    default=5558,
                    help='the projection server port')

parser.add_argument(
    '--resolution',
    type=int,
    default=512,
    help='the number of detector rows, detector columns, and angles')

args = parser.parse_args()

m = args.resolution
rows = m
cols = m
proj_count = m

pub = tomop.publisher(args.host, args.port)

# We let the server know we will send one dark field, and one flat field
packet_scan_settings = tomop.scan_settings_packet(0, 1, 1, False)
pub.send(packet_scan_settings)

# Initialize volume and acquisition geometry
packet_vol_geom = tomop.geometry_specification_packet(0, [0, 0, 0], [1, 1, 1])
pub.send(packet_vol_geom)

packet_geometry = tomop.parallel_beam_geometry_packet(
    0, rows, cols, proj_count, np.linspace(0, np.pi, proj_count))
pub.send(packet_geometry)

# Send dark(s) and flat(s)
packet_dark = tomop.projection_packet(0, 0, [rows, cols],
                                      np.zeros(rows * cols))
pub.send(packet_dark)

packet_flat = tomop.projection_packet(1, 0, [rows, cols],
                                      np.zeros(rows * cols))
pub.send(packet_flat)

# Create and send projection data consisting of zeros
proj_data = np.zeros((proj_count, rows, cols))
for i in np.arange(0, proj_count):
    packet_proj = tomop.projection_packet(
        2, i, [rows, cols], np.ascontiguousarray(proj_data[i, :, :].flatten()))
    pub.send(packet_proj)
