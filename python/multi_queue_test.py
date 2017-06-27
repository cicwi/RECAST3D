import tomop
import numpy as np
import threading
import time
from queue import Queue

serv = tomop.server("Multi-GPU scene")

vdp = tomop.volume_data_packet(
    serv.scene_id(),
    np.array([2, 2, 2], dtype='int32'),
    np.array([0, 255, 128, 255, 255, 128, 255, 0], dtype='float32'))

serv.send(vdp)

p = 4
queues = [Queue(), Queue(), Queue(), Queue()]


def gpu_with_queue(s, q, server):
    print('starting thread', s)

    def callback_fbp(t, orientation):
        data = np.zeros(16, dtype='float')
        data[t + 1] = t + 2
        return np.array([4, 4], dtype='int32'), data

    # initialize astra
    #.....
    while True:
        orientation, slice_id = q.get()
        print(s, slice_id)
        q.task_done()

        size, data = callback_fbp(s, orientation)
        slice_packet = tomop.slice_data_packet(server.scene_id(),
                                               slice_id, size, True, data)
        server.send(slice_packet)


# make p threads
threads = []
for i in range(p):
    threads.append(threading.Thread(target=gpu_with_queue, args=[i, queues[i],
                                                                 serv]))
    threads[i].start()


def callback(orientation, slice_id):
    for q in queues:
        q.put((orientation, slice_id))
    for q in queues:
        q.join()
    return [], []

serv.set_callback(callback)
serv.serve()
