import numpy as np
import slicerecon

def callback(size, xs, idx):
    xs = np.array(xs).reshape(size)

    print("callback called", size)
    xs[xs <= 3.0] = 0.0
    xs[xs > 3.0] = -10.0

    return [size, xs.ravel().tolist()]

p = slicerecon.plugin("tcp://*:5652", "tcp://localhost:5555")
p.set_slice_callback(callback)

p.listen()
