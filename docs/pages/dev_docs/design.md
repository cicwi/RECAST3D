# Design

The developer guide is intended for (future) contributors to the RECAST3D stack.

The reference implementation for the quasi-3D reconstruction stack is based on TomoPackets for communication, SliceRecon for reconstruction, and RECAST3D for visualization.

{% dot design_overview.svg

  digraph G {
    "Scanner" [label=<Scanner <br/><br/><i>Data Adapter</i>>, shape=rectangle, pos = "-10,0!"]
    "Cluster" [label=<Cluster <br/><br/><i>SliceRecon</i>>, shape=rectangle, pos = "0,0!"]
    "Workstation(s)" [label=<Workstations(s) <br/><br/><i>RECAST3D</i>>, shape=rectangle, pos = "10,0!"]

    "Scanner" -> "Cluster" [label=< <i>TomoPackets</i>>]
    "Cluster" -> "Workstation(s)" [dir="both", label=< <i>TomoPackets</i>>]
  }
%}

The three components have the following goals:

- *TomoPackets*
    - Hide the complexity of making servers and implementing communication
    - Standardize data for real-time tomographic imaging
- *SliceRecon*
    - Implement efficient quasi-3d reconstruction
    - Plugin system for research on real-time tomography
- *RECAST3D*
    - Allow an operator to visualize the object and interact during a tomographic scan


## Distributed approach

The stack has a fully distributed approach, meaning that any component can run on any node as long as the nodes are connected via a network. This has a number of advantages:

- Easily distribute work over clusters
- Extend real-time imaging pipeline using TomoPackets, automatically works with all components of the stack
- Able to switch underlying technology (replace SliceRecon, other (web-based, VR, …) visualizer, use something other than ZeroMQ, …)
- Make an adapter for another scanner (or electron microscope, detector at beamline, …) and the real-time imaging just works
- Visualize ‘remotely’, i.e. workstation instead of scan servers, or even visualize in the Netherlands while reconstruction runs in Switzerland without any noticeable lag


