import tomop
import numpy as np
import threading

class MiniGPUServer(threading.Thread):
    def __init__(self, group=None, target=None, name=None,
             server=None, s=None):
        threading.Thread.__init__(self, group=group, target=target, name=name)
        print('init')
        self.server = server
        self.orientation_queue = []
        self.event = threading.Event()
        self.event.clear()
        self.s = s
        return

    def run(self):
        print('run')
        while True:
            self.event.wait()
            print(self.s)
            # process orientations
            # and so on
            self.event.clear()


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
    threads[i].start()

def callback(orientation, slice_id):
    global threads
    print('callback')
    for thread in threads:
        thread.orientation_queue.append((orientation, slice_id))
        # make sure that event is not yet set, otherwise wait?!
        # alternative, use condition var
        thread.event.set()
    return [], []

serv.set_callback(callback)
serv.serve()
