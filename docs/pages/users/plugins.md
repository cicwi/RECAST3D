# Plugins

Currently, plugins are only post-processing on slices

## Developing a plugin

Developing a post-processing plugin is as easy as implementing a single Python
function that takes a 2D numpy array (the reconstructed slice), and returns a 2D
numpy array (the postprocessed slice). An example plugin looks like this.

```python
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
```


This plugin listens to incoming `SliceData` packets on port `5652`, and connects
to a visualization software (or another plugin) listening on port `5555`. These
are the default values. If you use the standard `slicerecon_server` program,
connecting the Python plugin is as easy as passing `--pyplugin` as a flag.

### Testing your plugin


1. Start RECAST3D:

```bash
recast3d
```

2. Start `slicerecon_server`, e.g.:

```bash
slicerecon_server --slice-size 512 --pyplugin
```

3. Run your plugin, e.g.:

```bash
python plugin.py
```

4. Stream projection data to the `slicerecon_server`, e.g.:

```bash
python slicerecon_push_flexdata.py [path_to_data] --sample 2
```

