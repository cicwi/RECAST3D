import numpy as np
import slicerecon
from skimage import filters

def callback(shape, xs, _):
    # The Otsu implementation does not accept only zeros, so we pass through
    if not np.array(xs).any():
        return [shape, xs]

    # Reshape into an image
    xs = np.array(xs).reshape(shape)

    # Compute a threshold using Otsu's method
    val = filters.threshold_otsu(xs)

    # Threshold the image accordingly
    xs[xs <= val] = 0.0
    xs[xs > val] = 1.0

    return [shape, xs.ravel().tolist()]

# Host a plugin on the default port, with the default RECAST3D endpoint
p = slicerecon.plugin("tcp://*:5652", "tcp://localhost:5555")

# Register the callback
p.set_slice_callback(callback)

# Start the plugin
p.listen()
