import math
import ser_parser
import os
import time
import numpy as np
import tomop
import argparse
#import matlab.engine
#import matplotlib.pyplot as plt
import scipy.signal as ss
from scipy import ndimage
import skimage as ski

parser = argparse.ArgumentParser(description='Push EMAT data to SliceRecon.')
parser.add_argument('path', metavar='path', help='path to the data')
parser.add_argument('--host',
                    default="localhost",
                    help='the projection server host')
parser.add_argument('--port',
                    type=int,
                    default=5558,
                    help='the projection server port')
parser.add_argument('--prerecorded',
                    action='store_true',
                    help='use prerecorded set')
parser.add_argument('--linearize',
                    action='store_true',
                    help='whether data is linear')
parser.add_argument('--angles', type=int, nargs='+', help='list of angles')
parser.add_argument('-a', type=int, help='angle start')
parser.add_argument('-b', type=int, help='angle end')
parser.add_argument('-i', type=int, help='increment')
parser.add_argument('-m', type=int, default=1024, help='number of rows')
parser.add_argument('-n', type=int, default=1024, help='number of columns')
args = parser.parse_args()

path = args.path
count = 0
files = []
angles = np.zeros((0))
m, n = 0, 0

if not args.prerecorded:
    m = args.m
    n = args.n
    # angles = args.angles
    angles = np.arange(args.a, args.b + 1, args.i)
    count = len(angles)
    files = list(
        map(lambda ang: (ang, path + "/" + str(ang) + "_1.ser"), angles))
else:

    def to_float(str):
        return float(str[:-2])

    for filename in os.listdir(path):
        if filename.endswith(".ser"):
            count += 1

    files = [(0, "")] * count
    angles = np.zeros(count)

    i = 0
    for filename in os.listdir(path):
        if filename.endswith(".ser"):
            full_path = os.path.join(path, filename)
            angles[i] = to_float(os.path.splitext(filename)[0])
            files[i] = (angles[i], full_path)
            i += 1

    (m, n), first_proj = ser_parser.parser(files[0][1])

#files = sorted(files)
#angles = np.sort(angles)
print(files)

angles = angles * (math.pi / 180.0)
print(angles)


def align(xs, ys):
    zs = ss.fftconvolve(xs, ys[::-1, ::-1])
    return np.array(np.unravel_index(np.argmax(zs, axis=None),
                                     zs.shape)) - np.array(xs.shape) + [1, 1]


pub = tomop.publisher(args.host, args.port)
path = args.path

# PACKET 1: object volume specification
print("Sending object volume")
geom_spec = tomop.geometry_specification_packet(0, [-n / 2, -n / 2, -n / 2],
                                                [n / 2, n / 2, n / 2])
pub.send(geom_spec)
# PACKET 2: acquisition geometry
print("Sending acquisition geometry")
par_beam = tomop.parallel_beam_geometry_packet(0, m, n, count, angles)
pub.send(par_beam)

# PACKET 3: scan settings
print("Sending scan data (linear: ", not args.linearize, ")")
pub.send(tomop.scan_settings_packet(0, 0, 0, not args.linearize))

# PACKET 4...: Projections
while not os.path.isfile(files[0][1]):
    time.sleep(0.01)

time.sleep(1.0)

shape, first_proj = ser_parser.parser(files[0][1])
prev = np.reshape(first_proj, [m, n])
prev = np.transpose(prev, axes=[1, 0])


def center(xs):
    val = ski.filters.threshold_otsu(xs)
    ys = xs.copy()
    ys[ys < val] = 0.0
    x, y = ndimage.measurements.center_of_mass(ys)
    shift = np.array([m // 2 - x, n // 2 - y]).round()
    return np.roll(xs, np.array(shift, dtype=np.int32), (0, 1))


prev = center(prev)


def convert(af):
    return (af[1][0], af[1][1], af[0])


files = list(map(convert, enumerate(files)))

while len(files) > 0:
    file_found = ""
    angle_found = 0
    idx_found = -1
    while True:
        for (angle, filename, proj_idx) in files:
            if os.path.isfile(filename):
                file_found = filename
                angle_found = angle
                idx_found = proj_idx
                break
            time.sleep(0.01)
        if (idx_found >= 0):
            break
        time.sleep(1.0)
    print((angle_found, file_found))
    files.remove((angle_found, file_found, idx_found))

    print("Sending projection: ", idx_found)
    shape, data = ser_parser.parser(file_found)
    xs = np.reshape(data, shape)
    xs = np.transpose(xs, axes=[1, 0])
    xs = center(xs)

    shift = align(xs, prev)
    print(shift)
    shifted_xs = np.roll(xs, -shift, (0, 1))

    pub.send(
        tomop.projection_packet(2, idx_found, [m, n],
                                np.ascontiguousarray(shifted_xs.ravel())))

    prev = shifted_xs
