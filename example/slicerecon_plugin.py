import numpy as np
import slicerecon


def callback(shape, xs, idx):
    xs = np.array(xs).reshape(shape)

    print("callback called", shape)
    xs[xs <= 3.0] = 0.0
    xs[xs > 3.0] = 10.0

    return [shape, xs.ravel().tolist()]


p = slicerecon.plugin("tcp://*:5652", "tcp://localhost:5555")
p.set_slice_callback(callback)

p.listen()
