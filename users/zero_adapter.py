import tomop
import numpy as np
import argparse


def push_zero_projections(resolution, host="localhost", port=5558):
    m = resolution
    proj_count, rows, cols = m, m, m
    scene_id = 0

    pub = tomop.publisher(host, port)

    # We let the server know we will send one dark field, and one flat field
    num_darks, num_flats = 1, 1
    packet_scan_settings = tomop.scan_settings_packet(scene_id, num_darks,
                                                      num_flats, False)
    pub.send(packet_scan_settings)

    # Initialize volume and acquisition geometry
    packet_vol_geom = tomop.geometry_specification_packet(
        scene_id, [0, 0, 0], [1, 1, 1])
    pub.send(packet_vol_geom)

    angles = np.linspace(0, np.pi, proj_count, endpoint=False)
    packet_geometry = tomop.parallel_beam_geometry_packet(
        scene_id, rows, cols, proj_count, angles)
    pub.send(packet_geometry)

    # Send dark(s) and flat(s)
    dark = np.zeros((rows, cols), dtype=np.float32).ravel()
    packet_dark = tomop.projection_packet(0, 0, [rows, cols], dark)
    pub.send(packet_dark)

    flat = np.ones((rows, cols), dtype=np.float32).ravel()
    packet_flat = tomop.projection_packet(1, 0, [rows, cols], flat)
    pub.send(packet_flat)

    # Create and send projection data consisting of zeros
    proj_data = np.zeros((proj_count, rows, cols))
    for i in np.arange(0, proj_count):
        packet_proj = tomop.projection_packet(2, i, [rows, cols],
                                              proj_data[i, :, :].ravel())
        pub.send(packet_proj)


if __name__ == '__main__':
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

    push_zero_projections(args.resolution, host=args.host, port=args.port)
