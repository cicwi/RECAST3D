import tomop
import numpy as np
import threading

class MiniGPUServer(threading.Thread):
    def __init__(self, group=None, target=None, name=None,
             server=None, s=None, verbose=None):
        threading.Thread.__init__(self, group=group, target=target, name=name,
                                  verbose=verbose)
        self.server = server
        self.orientation_queue = []
        self.event = threading.Event
        self.s = s
        return

    def run(self):
        while True:
            event.wait()
            # process orientations
            # and so on
            event.clear()


serv = tomop.server("hi")

vdp = tomop.volume_data_packet(
    serv.scene_id(),
    np.array([2, 2, 2], dtype='int32'),
    np.array([0, 255, 128, 255, 255, 128, 255, 0], dtype='float32'))

serv.send(vdp)

threads = []
p = 4

# make p threads
for i in range(p):
    threads.append(MiniGPUServer(server=serv, s=i))

def callback(orientation, slice_id):
    global threads
    for thread in threads:
        thread.orientation_queue.append((orientation, slice_id))
    thread.event.set()
    return [], []

serv.set_callback(callback)
serv.serve()
