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

<div class="mermaid">
  graph LR
    reconstructor[Reconstructor]
    plugin["Plugin(s)"]
    visualizer[Visualizer]

    visualizer-- set slice -->reconstructor
    reconstructor-. slice data.->visualizer
    reconstructor-- slice data -->plugin
    plugin-- slice data -->visualizer
</div>

There can be more than one plugin, but they are assumed to be applied one after
the other. The dashed line is only used if there are no plugins.

## Conventions

### Multi-dimensional arrays

- Volume data is stored in x-y-z order (from major to minor).
- Projection data is stored in row-column order (from major to minor).


## SliceRecon architecture

An overview of the design of SliceRecon is as follows.

<div class="mermaid">
    graph TB
      projection_data[(Projection Data)]
      recast3d[RECAST3D]
      projection_server[/projection_server/]
      reconstructor[/reconstructor/]
      buffer1[inactive buffer]
      buffer2[active buffer]
      solver[/solver/]
      visualization_server[/visualization_server/]

      projection_data-->projection_server

      subgraph SliceRecon
      projection_server-- scan settings -->reconstructor
      projection_server-- geometry -->reconstructor
      projection_server-. projections .->reconstructor

      reconstructor-- preprocessed projections -->buffer1

      buffer1-- swap -->buffer2
      buffer2-- swap -->buffer1

      buffer2-->solver

      visualization_server-- requests/configuration -->solver
      solver-- reconstructions -->visualization_server
      end

      visualization_server --> recast3d
</div>

When projection data comes into SliceRecon, it gets put into an ‘inactive buffer’.
As soon as enough projection data is processed and in main RAM, we ‘upload’ to the GPU. This happens in a number of steps:

- pre-process
    - flat fielding
    - FDK scaling
    - filtering
    - phase retrieval
    - ...
- transpose sinogram
- upload to GPU

There are two modes, _alternating_ and _continuous_. In ‘alternating’ mode, we always reconstruct from the last complete set of projections. In ‘continuous mode’ we reconstruct with for each projection the most recent data.

## Data flowing in/out of SliceRecon

<div class="mermaid">
    classDiagram
      class Adapter {
          scan settings
          vol_geom
          proj_geom
          [projection]
      }
      class User Settings {
        slice_size
        preview_size
        group_size
        filter_cores
      }
      class projection_server
      class visualization_server
      class solver
      class Plugins
      class RECAST3D

      projection_server <|.. Adapter
      projection_server <|-- User Settings

      visualization_server <|-- solver : reconstructions
      solver <|-- projection_server

      Plugins <|.. visualization_server : reconstructions
      RECAST3D <|.. Plugins

      visualization_server <|.. RECAST3D : requests

      solver <|-- visualization_server : requests
</div>

Dashed lines happen within SliceRecon, while solid lines are communicated using TomoPackets.

## Solver implementation

The solver can reconstruct an arbitrarily oriented slice from the full 3D projection data. 

Instead of considering the full 3D volume, we setup our geometry by constructing an object volume that consists of the central axial slice $C$ only. If we want to reconstruct an arbitrary slice $S$, we can transform $S$ into $C$ using a combination of a translation vector $\delta$ from the center of slice $S$ onto the center of $C$ (and thus the full 3D volume), a rotation $\mathcal{R}$, and optionally a scale factor which does not have to be used when slices are of fixed size.

![Slice transformation](../images/slice_transformation.svg)

For a cone-beam geometry, we can define each projection by a source position $\vec{s}$, a detector position $\vec{d}$, and two axes $\vec{u}$ and $\vec{v}$ that define pixel distances on the detector.

We then transform each projection according to:

$$\begin{align*}
\vec{s}' &= \mathcal{R} (\vec{s} + \delta) \\
\vec{d}' &= \mathcal{R} (\vec{d} + \delta) \\
\vec{u}' &= \mathcal{R} (\vec{u}) \\
\vec{v}' &= \mathcal{R} (\vec{v})
\end{align*}$$

If we then reconstruct with the transformed geometry, we are effectively using a geometry in which the arbitrary slice $S$ has become the central slices, without adjusting projection data. This is the basic idea behind the solver implementations: we adjust the geometry on the fly for each slice that we are interested in, and run a standard backprojection algorithm.


