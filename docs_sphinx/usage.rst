Usage
=====

Running a reconstruction server
-------------------------------

Before running the reconstruction server, start up a visualization server like
RECAST3D_. To run the server::

  ./slicerecon_server [options]

The server is now waiting until (projection) data is pushed to it. For example,
we can push prerecorded FleX-ray data::

  python slicerecon_push_flexray.py [path]

Developing a plugin
-------------------

Developing a post-processing plugin is as easy as implementing a single Python
function that takes a 2D numpy array (the reconstructed slice), and returns a 2D
numpy array (the postprocessed slice). An example plugin looks like this.

.. code:: python

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


This plugin listens to incoming `SliceData` packets on port `5652`, and connects
to a visualization software (or another plugin) listening on port `5555`. These
are the default values. If you use the standard `slicerecon_server` program,
connecting the Python plugin is as easy as passing `--pyplugin` as a flag.

Testing your plugin
~~~~~~~~~~~~~~~~~~~

1. Start RECAST3D::

    module load recast3d
    recast3d

2. Start `slicerecon_server`, e.g.::

     slicerecon_server --slice-size 512 --pyplugin

3. Run your plugin, e.g.::

     python plugin.py

4. Stream projection data to the `slicerecon_server`, e.g.::

     python slicerecon_push_flexdata.py [path_to_data] --sample 2

.. _RECAST3D: https://www.github.com/cicwi/RECAST3D
