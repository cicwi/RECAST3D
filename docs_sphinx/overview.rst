Overview
========

The SliceRecon project defines three main objects:

- A projection server, that listens to incoming projection data.
- A reconstructor, that can reconstruct arbitrarily oriented slices from
  projection data.
- A visualization server, that listens to requests from a visualization server,
  and fulfils them by calling the reconstructor.

Furthermore, it has a notion of a plugin, which is a stand alone server that can
postprocess reconstructed slices before sending them to the visualization
server.

The incoming, internal, and outgoing communication is all handled by the TomoPackets_ library.

Projection server
-----------------

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

Reconstructor
-------------

The reconstructor is an internal object that decouples the projection server
from the visualization server, and has no public interface. It receives
projection data from the projection server, and fulfills reconstruction requests
from the visualization server.

Visualization server
--------------------

The visualization server registers itself to the visualization software by
sending a `MakeScene` packet. It then waits to receive `KillScene`, `SetSlice`
and `RemoveSlice` packets. If it receives a `SetSlice` packet, it requests a new
slice reconstruction from the reconstructor. It sends this reconstructed slice
back either to the visualization software using a `SliceData` packet if there
are no active plugins, or to the first plugin.

Plugin
------

A *plugin* is a simple server, that registers itself to the visualization server,
and listens to incoming `SliceData` packets. It then manipulates the data in
this `SliceData` packet, before sending it along to the next plugin in line, or
to the visualization software. The plugin system thus has the following structure:

.. graphviz::

  digraph G {
    "Reconstructor" [pos = "0,10!"]
    "Plugin(s)" [shape=diamond, pos = "0,0!"]
    "Visualizer" [pos = "0,-10!"]

    "Reconstructor" -> "Visualizer" [style="dashed", label="SliceData"]
    "Visualizer" -> "Reconstructor" [label="SetSlice"]
    "Reconstructor" -> "Plugin(s)" [label="SliceData"]
    "Plugin(s)" -> "Visualizer" [label="SliceData"]
  }

There can be more than one plugin, but they are assumed to be applied one after
the other. The dashed line is only used if there are no plugins.

.. _TomoPackets: https://www.github.com/cicwi/TomoPackets
