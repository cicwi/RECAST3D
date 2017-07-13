import tomop
import numpy as np
import threading
import time


server_lock = threading.Lock()


class MiniGPUServer(threading.Thread):
    def __init__(self, group=None, target=None, name=None,
                 server=None, s=None):
        threading.Thread.__init__(self, group=group, target=target, name=name)

        # set instance variables
        self.server = server
        self.orientation_queue = []
        self.cv = threading.Condition()
        self.s = s

        # call setup function
        self.construct_geometry()

        return


    def construct_geometry(self):
        pass


    def callback_fbp(self, orientation):
        data = np.zeros(16, dtype='float')
        data[self.s] = self.s + 1;
        return [4, 4], data


    def run(self):
        # keep scanning for events
        while True:
            with self.cv:
                while not self.orientation_queue:
                    self.cv.wait()
                orientation, slice_id = self.orientation_queue.pop(0)
                size, data = self.callback_fbp(orientation)

                print(time.time(), self.s, 'sending packet', slice_id)
                slice_packet = tomop.slice_data_packet(self.server.scene_id(),
                                                       slice_id, size, True, data)
                self.server.send(slice_packet)

serv = tomop.server("Multi-GPU scene")

vdp = tomop.volume_data_packet(
    serv.scene_id(),
    [2, 2, 2],
    np.array([0, 255, 128, 255, 255, 128, 255, 0], dtype='float32'))

serv.send(vdp)

threads = []
p = 4

# make p threads
for i in range(p):
    threads.append(MiniGPUServer(server=serv, s=i))
    threads[i].start()


def callback(orientation, slice_id):
    print('enter callback')
    global threads
    for thread in threads:
        with thread.cv:
            thread.orientation_queue.append((orientation, slice_id))
            thread.cv.notify()
    print('exit callback (all locks released...)')
    return [], []

serv.set_callback(callback)
serv.serve()
