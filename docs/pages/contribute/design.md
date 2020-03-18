# Design

This developer guide is intended for (future) contributors to the RECAST3D stack.

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

Hopefully, after reading these pages, you will have a good understanding of the overall design of the live reconstruction stack.

## Distributed approach

A distinguishing feature of the stack is that we have taken a fully distributed approach, meaning that any component can run on any node as long as the nodes are connected via a network. This has a number of advantages:

- We can easily distribute work over clusters
- We can extend the real-time imaging pipeline using TomoPackets, so that it automatically works with all components of the stack
- Will be able to switch the underlying technology: each component can be replaced, and new (web-based, VR, …) visualizers can e developed. We can even decide to change the underlying messaging service to something other than ZeroMQ.
- It is easy to make the stack work for any data source, by writing an adapter for another scanner (or electron microscope, detector at beamline, etc.). The real-time imaging should work all the same.
- We are able to visualize ‘remotely’, i.e. on a workstation instead of the terminal attached to the imaging device. We have even shown the feasibility of visualizing in the Netherlands, while reconstruction runs in Switzerland, all without any noticeable lag.

