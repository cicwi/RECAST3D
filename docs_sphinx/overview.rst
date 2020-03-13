========
Overview
========

Introduction
------------

This library implements a communication protocol for a distributed tomographic
reconstruction pipeline in which parameters used in the reconstruction can be
changed in real-time, taking effect on the running reconstruction code
immediately.

The protocol is based on the reconstruction of individual slices, for example
orthogonal planes, and is useful for situations where the projection data is to
big to reconstruct completely in real-time. The slices are shown together in a
3D interface, and get updated when, for example:

- new (projection) data is available
- more iterations for iterative solvers have been applied
- higher resolution reconstructions are available

The position and orientation of the *active slices* can be changed, and this is
communicated back to the reconstruction cluster, which for future updates will
then reconstruct these new slices.



Pipeline
--------

For real-time imaging experiments, there are many nodes at work at the same
time. A rough overview of the topology that is recommended (but not required)
for communication based on TomoPackets is as follows.

.. graphviz::

    digraph G {
      "Scan Data" -> "Adapter"
      "Adapter" -> "Reconstructor"
      "Reconstructor" -> "Visualizer" 
      "Visualizer" -> "Reconstructor" 
      "Reconstructor" -> "Plugin"
      "Plugin" -> "..."
      "..." -> "Visualizer"
    }

We have the following node types.

- **Scan Data**. The entry point of the pipeline: data coming from the detector
  which get pushed into the network. This data can also come from simulations or
  be prerecorded.

- **Adapter**. This node is specific to the projection data, and metadata about
  the acquistion. It converts this application specific data, to a common format
  that can be used by nodes further down the pipeline that implement the
  TomoPackets protocol. Some adapter examples can be found in the slicerecon_
  project.

- **Reconstructor**. This node receives projection data and metadata, and uses it
  it to fulfil reconstruction requests from visualization nodes further down the
  line. The slicerecon_ project implements such a node.

- **Plugins**. Plugin nodes take reconstructed slices, and postprocess them. For
  example, for real-time segmentation, artefact removal, other image
  enhancements, or quantitative analysis of the imaged object. If there are no
  active plugins, the reconstructed slice given by the reconstructor is sent
  directly to the visualization node. Example plugins can also be found in the
  slicerecon_ project.

- **Visualizer**. The visualization node shows the reconstructions to a user. When
  the user changes the slices that are being reconstructed, the visualizer
  requests a new reconstruction from the reconstructor. An example visualizer is
  the RECAST3D_ software.

Packets
-------

The communication between nodes happens in a using standardized *network packets*
that contain data, commands, or parameters. Here we give some examples of these
packets.

Adapter to Reconstructor
~~~~~~~~~~~~~~~~~~~~~~~~

The reconstructor needs to receive three types of information from the data
adapter.

- Information about where the object is positioned in relation to the
  acquisition geometry. This is done using a
  :class:`tomop.geometry_specification_packet`, which defines the minimum and
  maximum point of a bounding box around the object, which is the physical
  region to be reconstructed.

- Information of the acquisition geometry. This is done using either of the
  following four packets.

  - :class:`tomop.parallel_beam_geometry_packet`
  - :class:`tomop.parallel_vec_geometry_packet`
  - :class:`tomop.cone_beam_geometry_packet`
  - :class:`tomop.cone_vec_geometry_packet`

- The projection data. This is done using (multiple)
  :class:`tomop.projection_packet`. The `type` field of his packet denotes dark
  (`0`), bright (`1`), or ordinary (`2`) projections.

Reconstructor to and from Visualizer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The communication between the reconstructor and visualizer uses the following
packets.

- To construct a scene, a :class:`tomop.make_scene_packet` is sent to the
  visualizer. This is done over a `REQ/REP` channel, the reply is the assigned
  `scene_id` which can be used to tag later packets.

- After the scene is constructed, the reconstructor waits to receive
  :class:`tomop.set_slice_packet` requests.

- It responds to these packets using a :class:`tomop.slice_data_packet`. If
  there are plugins active, this packet is sent to the first plugin in line,
  which sends it to the next plugin after it is done processing. The final
  plugin then sends it to the visualizer.

.. _slicerecon: https://www.github.com/cicwi/slicerecon
.. _RECAST3D: https://www.github.com/cicwi/recast3d
