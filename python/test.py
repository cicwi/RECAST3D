import tomop
import numpy as np


def callback(orientation, slice_id):
    print("callback called")
    print(orientation)
    return np.array([4, 4], dtype='int32'), np.array([0, 255, 0, 255, 255, 0, 255, 0, 255,
                                                      0, 0, 255, 255, 0, 0,
                                                      255], dtype='float32')

serv = tomop.server("hi")

vdp = tomop.volume_data_packet(
    serv.scene_id(),
    np.array([2, 2, 2], dtype='int32'),
    np.array([0, 255, 128, 255, 255, 128, 255, 0], dtype='float32'))

serv.send(vdp)

serv.set_callback(callback)
serv.serve()
