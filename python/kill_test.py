import tomop
import numpy as np

serv = tomop.server("hi")
SCENE_ID = serv.scene_id()
del serv

def callback(orientation, slice_id):
    print("callback called")
    print(orientation)
    return np.array([4, 4], dtype='int32'), np.array([0, 255, 128, 255, 255, 128, 255, 0, 255,
                                                      0, 128, 255, 255, 128, 0,
                                                      255], dtype='float32')

serv = tomop.server(SCENE_ID)
serv2 = tomop.server(SCENE_ID)
serv3 = tomop.server(SCENE_ID)
serv4 = tomop.server(SCENE_ID)

serv.set_callback(callback)
serv2.set_callback(callback)
serv3.set_callback(callback)
serv4.set_callback(callback)

serv2.listen()
serv3.listen()
serv4.listen()
serv.serve()
