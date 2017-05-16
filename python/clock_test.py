import tomop
import numpy as np


def pack(data, norm):
    return np.array((255 / norm) * data, dtype='uint8')


def slice_data(data, orientation, slice_shape):
    norm = np.max(data)

    dy = np.array([orientation[0], orientation[1], orientation[2]])
    dx = np.array([orientation[3], orientation[4], orientation[5]])
    base = np.array([orientation[6], orientation[7], orientation[8]])

    i = np.arange(0, slice_shape[0])
    j = np.arange(0, slice_shape[1])

    points = np.empty([slice_shape[0], slice_shape[1], 3])
    points[:] = base[None, None, :] + (1.0 / (slice_shape[0] - 1)) * i[:, None, None] * \
        dx[None, None, :] + (1.0 / (slice_shape[1] - 1)) * \
        j[None, :, None] * dy[None, None, :]
    points = data.shape * (0.5 * (points + 1.0))

    points[points < 0] = 0
    points[points >= slice_shape[0]] = slice_shape[0] - 1
    indices = points.astype(int)

    data_to_return = data[[indices[..., 0], indices[..., 1], indices[..., 2]]]

    return pack(data_to_return, norm)


def main():
    size = 1024

    space = np.load(
        '/export/scratch2/buurlage/rec_animated_gears_broken_I10000.npy')
    space = space - np.min(space)
    space[space > 10.0] = 10.0
    space[space < 9.8] = 0.0

    def callback(orientation):
        slice_shape = np.array([1024, 1024], dtype='int32')
        slice_payload = slice_data(space, orientation, slice_shape)
        print(slice_payload.shape)
        return slice_shape, slice_payload.ravel()

    serv = tomop.server("3D data test")

    small_size = 128
    stride = size // small_size

    vdp = tomop.volume_data_packet(
        serv.scene_id(),
        np.array([small_size, small_size, small_size], dtype='int32'),
        pack(np.transpose(space[::stride, ::stride, ::stride], axes=[2, 1, 0]).ravel(), np.max(space)))

    serv.send(vdp)

    serv.set_callback(callback)
    serv.serve()

main()
