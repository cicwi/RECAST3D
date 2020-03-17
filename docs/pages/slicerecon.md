# SliceRecon

## Overview

The SliceRecon project defines three main objects:

- A projection server, that listens to incoming projection data.
- A reconstructor, that can reconstruct arbitrarily oriented slices from
  projection data.
- A visualization server, that listens to requests from a visualization server,
  and fulfils them by calling the reconstructor.

Furthermore, it has a notion of a plugin, which is a stand alone server that can
postprocess reconstructed slices before sending them to the visualization
server.

The incoming, internal, and outgoing communication is all handled by the TomoPackets library.

### Projection server

The projection server listens for incoming data packets. It expects first
packets that describe the tomographic scan. This is done using:

- `GeometrySpecification`: information on where the object is in relation
  to the acquisition geometry.
- `ScanSettings` packet: information on the number of darks and flats.
- A packet describing the acquisition geometry, such as a `ConeVecGeometry`
  packet.

After receiving these packets, the server is able to process `ProjectionData`
packets. First the darks and flats should be sent, after which standard
projections can be streamed to the projection server.

### Reconstructor

The reconstructor is an internal object that decouples the projection server
from the visualization server, and has no public interface. It receives
projection data from the projection server, and fulfills reconstruction requests
from the visualization server.

### Visualization server

The visualization server registers itself to the visualization software by
sending a `MakeScene` packet. It then waits to receive `KillScene`, `SetSlice`
and `RemoveSlice` packets. If it receives a `SetSlice` packet, it requests a new
slice reconstruction from the reconstructor. It sends this reconstructed slice
back either to the visualization software using a `SliceData` packet if there
are no active plugins, or to the first plugin.

### Plugin

A *plugin* is a simple server, that registers itself to the visualization server,
and listens to incoming `SliceData` packets. It then manipulates the data in
this `SliceData` packet, before sending it along to the next plugin in line, or
to the visualization software. The plugin system thus has the following structure:

{% dot slicerecon_overview.svg

  digraph G {
    "Reconstructor" [pos = "0,10!"]
    "Plugin(s)" [shape=diamond, pos = "0,0!"]
    "Visualizer" [pos = "0,-10!"]

    "Reconstructor" -> "Visualizer" [style="dashed", label="SliceData"]
    "Visualizer" -> "Reconstructor" [label="SetSlice"]
    "Reconstructor" -> "Plugin(s)" [label="SliceData"]
    "Plugin(s)" -> "Visualizer" [label="SliceData"]
  }
%}

There can be more than one plugin, but they are assumed to be applied one after
the other. The dashed line is only used if there are no plugins.

## Conventions

### Multi-dimensional arrays

- Volume data is stored in x-y-z order (from major to minor).
- Projection data is stored in row-column order (from major to minor).


## Usage

### Running a reconstruction server

Before running the reconstruction server, start up a visualization server like
RECAST3D_. To run the server:

```bash
  ./slicerecon_server [options]
```

The server is now waiting until (projection) data is pushed to it. For example,
we can push prerecorded FleX-ray data::

```bash
  python slicerecon_push_flexray.py [path]
```

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
