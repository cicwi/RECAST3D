import tomop
import numpy as np
import math


def rotation_matrix(axis, theta):
    """
    Return the rotation matrix associated with counterclockwise rotation about
    the given axis by theta radians.
    """
    axis = np.asarray(axis)
    axis = axis / math.sqrt(np.dot(axis, axis))
    a = math.cos(theta / 2.0)
    b, c, d = -axis * math.sin(theta / 2.0)
    aa, bb, cc, dd = a * a, b * b, c * c, d * d
    bc, ad, ac, ab, bd, cd = b * c, a * d, a * c, a * b, b * d, c * d
    return np.array([[aa + bb - cc - dd, 2 * (bc + ad), 2 * (bd - ac)],
                     [2 * (bc - ad), aa + cc - bb - dd, 2 * (cd + ab)],
                     [2 * (bd + ac), 2 * (cd - ab), aa + dd - bb - cc]])

def pack(data, norm):
    return np.array(data, dtype='float32')

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
    space = np.load(
        '/export/scratch2/buurlage/rec_sirt_animated_gears_broken_I5000_cleaned.npy')

    print("Loaded reconstruction..")

    def callback(orientation, slice_id):
        slice_shape = np.array([1000, 1000], dtype='int32')
        slice_payload = slice_data(space, orientation, slice_shape)
        return slice_shape, slice_payload.ravel()

    serv = tomop.server("3D data test")

    print("Started server..")

    small_size = 100
    stride = 1000 // small_size

    vdp = tomop.volume_data_packet(
        serv.scene_id(),
        np.array([small_size, small_size, small_size], dtype='int32'),
        pack(np.transpose(space[::stride, ::stride, ::stride], axes=[2, 1, 0]).ravel(), np.max(space)))

    serv.send(vdp)

    print("Sent volume data..")

    gsp = tomop.geometry_specification_packet(
        serv.scene_id(),
        False,
        1000)
    serv.send(gsp)

    source = [-11.0, 0.0, 0.0]
    detector_base = [1.5, 0.0, 0.0]
    detector_axes = [[0.0, 0.0, 1.0], [0.0, 1.0, 0.0]]
    projections = np.load(
        '/export/scratch2/buurlage/radio_animated_gears_broken_I5000_cleaned.npy')

    print("Loaded projections..")

    center = np.array([0.5, 0.5, 0.5])

    for i in np.arange(0, projections.shape[1], 10):
        rot = rotation_matrix([0, 1, 0], i * 0.002 * math.pi)
        proj = projections[:, i, :]
        rot_source = (np.dot(rot, source - center) + center).tolist()
        rot_a1 = np.dot(rot, detector_axes[0]).tolist()
        rot_a2 = np.dot(rot, detector_axes[1]).tolist()
        rot_base = (np.dot(rot, detector_base - center) + center).tolist()
        proj = np.flip(proj, 0)
        pdp = tomop.projection_data_packet(
            serv.scene_id(), i, rot_source,
            rot_a1 + rot_a2 + rot_base, [proj.shape[0],
                                         proj.shape[1]],
            pack(proj.flatten(), np.max(proj)))
        serv.send(pdp)

    print("Sent projections..")

    serv.set_callback(callback)

    print("Serving..")

    serv.serve()

main()
