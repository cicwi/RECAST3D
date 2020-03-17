# TomoPackets

## Introduction

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


## Pipeline

For real-time imaging experiments, there are many nodes at work at the same
time. A rough overview of the topology that is recommended (but not required)
for communication based on TomoPackets is as follows.

{% dot tomopackets_overview.svg
    digraph G {
      "Scan Data" -> "Adapter"
      "Adapter" -> "Reconstructor"
      "Reconstructor" -> "Visualizer" 
      "Visualizer" -> "Reconstructor" 
      "Reconstructor" -> "Plugin"
      "Plugin" -> "..."
      "..." -> "Visualizer"
    }
%}

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

## Packets

The communication between nodes happens in a using standardized *network packets*
that contain data, commands, or parameters. Here we give some examples of these
packets.

### Adapter to Reconstructor

The reconstructor needs to receive three types of information from the data
adapter.

- Information about where the object is positioned in relation to the
  acquisition geometry. This is done using a
  `tomop.geometry_specification_packet`, which defines the minimum and
  maximum point of a bounding box around the object, which is the physical
  region to be reconstructed.

- Information of the acquisition geometry. This is done using either of the
  following four packets.

  - `tomop.parallel_beam_geometry_packet`
  - `tomop.parallel_vec_geometry_packet`
  - `tomop.cone_beam_geometry_packet`
  - `tomop.cone_vec_geometry_packet`

- The projection data. This is done using (multiple)
  `tomop.projection_packet`. The `type` field of his packet denotes dark
  (`0`), bright (`1`), or ordinary (`2`) projections.

### Reconstructor to/from Visualizer

The communication between the reconstructor and visualizer uses the following
packets.

- To construct a scene, a `tomop.make_scene_packet` is sent to the
  visualizer. This is done over a `REQ/REP` channel, the reply is the assigned
  `scene_id` which can be used to tag later packets.

- After the scene is constructed, the reconstructor waits to receive
  `tomop.set_slice_packet` requests.

- It responds to these packets using a `tomop.slice_data_packet`. If
  there are plugins active, this packet is sent to the first plugin in line,
  which sends it to the next plugin after it is done processing. The final
  plugin then sends it to the visualizer.

.. _slicerecon: https://www.github.com/cicwi/slicerecon
.. _RECAST3D: https://www.github.com/cicwi/recast3d


## Conventions

### Multi-dimensional arrays

- Volume data is stored in x-y-z order (from major to minor).
- Projection data is stored in row-column order (from major to minor).

### Slice orientation

We need a convention for representing the orientation of a slice. The
orientation is inside *volume space* and is completely independent from the
number of pixels (i.e. the 'size' of an individual pixel is implied by the
bounding square of a slice). We represent the orientation as 9 real numbers
$(a, b, \ldots, i)$ so that:

$$
  \begin{equation}
  \begin{pmatrix}
  a & d & g \\
  b & e & h \\
  c & f & i \\
  0 & 0 & 1
  \end{pmatrix}
  \begin{pmatrix} x_s \\ y_s \\ 1 \end{pmatrix} =
  \begin{pmatrix} x_w \\ y_w \\ z_w \\ 1 \end{pmatrix}
  \end{equation}
$$

Where the vector $\vec{x}_s = (x_s, y_s)$ lives inside a slice (i.e. the
*normalized* pixel coordinates of a slice, in the interval $[0, 1]$), and where
$\vec{x}_w = (x_w, y_w, z_w)$ lives inside the volume geometry at the correct
place. The pixel coordinates of a slice have the following convention:

$$
  \begin{equation}
  \begin{pmatrix}
  (0, m) & \cdots & (n, m) \\
  \vdots & \ddots & \vdots \\
  (0, 0) & \cdots & (n, 0)
  \end{pmatrix}
  \end{equation}
$$

i.e. we start counting from the bottom-left and use a standard cartesian
$xy$ convention.

Using this convention, the vector $\vec{b} = (g, h, i)$ is the base point
of the slice in world space (i.e. the world coordinates of the bottom left point
of a slice). $\vec{x} = (a,b,c)$ is the direction in world space
corresponding to the $x$ direction of the slice, and $\vec{y} = (d,
e, f)$ corresponds to the $y$ direction.

## Acquisition geometries

The fields of the acquisition geometry packets:

- `tomop.parallel_beam_geometry_packet`
- `tomop.parallel_vec_geometry_packet`
- `tomop.cone_beam_geometry_packet`
- `tomop.cone_vec_geometry_packet`

follow the conventions of the constructors of 3D Geometries of the ASTRA
Toolbox. See <http://www.astra-toolbox.com/docs/geom3d.html> for an overview.


## Usage

### Writing an adapter

To implement an adapter, we have to send three types of packets to a listening
reconstructor. To send these packets, we can use a `tomop.publisher`
object. An example is provided below::

```python
  import tomop

  pub = tomop.publisher(host, port)

  packet_vol_geom = tomop.geometry_specification_packet(...)
  pub.send(packet_vol_geom)

  packet_geometry = tomop.cone_vec_packet(...)
  pub.send(packet_geometry)

  packet_dark = tomop.projection_packet(0, 0, [rows, cols], avg_dark)
  pub.send(packet_dark)

  packet_bright = tomop.projection_packet(0, 0, [rows, cols], avg_bright)
  pub.send(packet_bright)

  for i in np.arange(0, proj_count):
      packet_proj = tomop.projection_packet(2, i, [rows, cols], projection(i))
      pub.send(packet_proj)
```


### Writing a reconstruction node

For writing a simple reconstructor that responds to slice reconstruction
requests, you can use `tomop.server` and set a callback. You can also
send other types of packets over the channel opened by this server. For example,
sending a `tomop.volume_data_packet` enables a visualizer to show a 3D
preview::

```python
  import tomop
  import numpy as np
  
  
  def callback(orientation, slice_id):
      print("callback called")
      print(orientation)
      return [4, 4], np.array([0, 255, 0, 255, 255, 0, 255, 0, 255,
                               0, 0, 255, 255, 0, 0,
                               255], dtype='float32')
  
  serv = tomop.server("scene name")
  
  vdp = tomop.volume_data_packet(
      serv.scene_id(),
      np.array([2, 2, 2], dtype='int32').tolist(),
      np.array([0, 255, 128, 255, 255, 128, 255, 0], dtype='float32'))
  
  serv.send(vdp)
  
  serv.set_callback(callback)
  serv.serve()
```
