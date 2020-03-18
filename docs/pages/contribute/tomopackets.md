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

A packet is a group of data that has to be sent together. These can be 2D reconstructions, 3D reconstructions, but also slice reconstruction request, and so on. The TomoPackets implementation automates a lot of boilerplate for the developer, and automatically takes care of a lot things such as:

- Serializing
- Deserializing
- ‘Measuring’
- Sending over network
- Generating Python bindings with docs.

This required some template magic, which luckily is not necessary to understand in order to make new packets.

```cpp
struct Packet {
    // ...
    send(zmq::socket_t& socket);
    std::size_t size() const;
    memory_buffer serialize(int size) const;
    void deserialize(memory_buffer buffer);
};
```

TomoPackets is a flexible system, high-performance, easy to extend, and automatically ready to be used in real-time experiments. It abstracts away the whole ‘distributed part’ of the real-time reconstruction pipeline, so that adding features are just as easy as if it was all local. Automatically serialize/deserialize, send over network, and use other languages like Python in the real-time stack. Has built-in servers so that (in principle) clients do not have to rely explicitly on ZeroMQ.

### Descriptors

Each packet has a ‘descriptor’, which is a number to identify the content of the packet. The descriptors can be found in `descriptors.hpp`.

```cpp
enum class packet_desc : int {
   // SCENE MANAGEMENT
   make_scene = 0x101,
   kill_scene = 0x102,


   // RECONSTRUCTION
   slice_data = 0x201,
   partial_slice_data = 0x202,
   volume_data = 0x203,
   ...
```

### Defining a new packet

A new packet is defined using code such as the following.

```cpp
struct RegularizationParameterPacket : public PacketBase<RegularizationParameterPacket> {
   static const auto desc = packet_desc::regularization_parameter;
   RegularizationParameterPacket() = default;
   RegularizationParameterPacket(int32_t a, float b) : scene_id(a), lambda(b) {}
   BOOST_HANA_DEFINE_STRUCT(RegularizationParameterPacket, (int32_t, scene_id), (float, lambda));
};
```

This might seem more complex compared to defining a simple struct, such as:

```cpp
struct RegularizationParameterPacket : public Packet {
    int32_t scene_id;
    float lambda;
       packet_desc desc = packet_desc::regularization_parameter;
};
```

However, we get a lot in return. Code is automatically generated for networking (serialization, deserialization, measuring, sending, etc.), Python bindings, and the documentation.


We have to perform two additional actions: add ‘regularization_parameter’ to descriptor enum, and optionally add it to the list of packets to expose to Python in `tomop/module.cpp`


#### Serialization

Packets can contain components that are trivially copyable, and `std::string` (Python strings) and `std::vector` (Python lists / numpy arrays). 

To make a packet with a more exotic component, specialize `operator<<` and `operator>>` for `memory_span` in `serialize.hpp`.

### Hierarchy of packet classes / implementation

A schematic overview of the relevant classes and funtionality for the packets is as follows.

{% dot tomopackets_hierarchy.svg

  digraph G {
    "DerivedPacket" [label=<DerivedPacket>, shape=rectangle]
    "PacketBase" [label=<PacketBase[DerivedPacket]<br/><font color="darkgreen">serialize<br/>deserialize<br/>size</font>>, shape=rectangle]
    "Packet" [label="Packet", shape=rectangle]
    "omembuf" [label=<omembuf>]
    "imembuf" [label=<imembuf>]
    "memory_buffer" [label=<memory_buffer>, shape=rectangle]
    "fill"
    "scale" [shape=rectangle]

    "fill" -> "PacketBase" [style=dashed]
    "DerivedPacket" -> "PacketBase"
    "PacketBase" -> "Packet"
    "memory_buffer" -> omembuf
    "memory_buffer" -> imembuf
    "omembuf" -> Packet [style=dashed]
    "imembuf" -> Packet [style=dashed]
    "scale" -> Packet [style=dashed]
  }
%}

Here, solid lines denote inheritence, and dotted lines mean the source is used by the target.

The magic that allows everything to be generated automatically, is the `fill` function:

```cpp
template <typename Derived, typename Buffer>
void fill(Derived& base, Buffer& buffer) {
   hana::for_each(hana::accessors<Derived>(),
                  [&](auto pair) { buffer | hana::second(pair)(base); });
}
```

Every struct for a given functionality (serializing, deserializing and so on) overloads `operator|`, and this function iteratively calls this operator for each member of the packet. This is also the way the Python bindings are setup. _No manual code required_.

### Sending and receiving packets

Each packet has a `send(socket)` function that allows it to be sent over any ZeroMQ socket.

TomoPackets also has a built-in ‘server’. A `tomop::server` actually runs two independent ‘servers’: one for receiving projections, and one reconstruction server to respond to slice requests This is also where the descriptors come in: anything but `packet_desc::set_slice, packet_desc::remove_slice, packet_desc::kill_scene` are for example ignored by the ‘reconstruction’ server, while the projection server only listens to `packet_desc::projection_data`.

You can use: `set_slice_callback(callback_type callback)` for adding custom reconstruction code and `set_projection_callback(projection_callback_type callback)` to handle new projections from the scanner.

There is also a ‘multiserver’, which allows connections to more than one visualization tool (RECAST3D).

It is also possible to make custom servers which is what SliceRecon and RECAST3D do:

- Make a ZeroMQ socket to receive messages
- Read the descriptor, and deserialize the message based on the descriptor. After this, handle the packet using its contents.

Example code for custom server:

```cpp
socket_.recv(&update);
auto desc = ((tomop::packet_desc*)update.data())[0];
switch (desc) {
case tomop::packet_desc::scan_settings: {
  auto mbuffer = tomop::memory_buffer(update.size(),
                                    (char*)update.data());
  auto packet =
  std::make_unique<tomop::ScanSettingsPacket>();
  packet->deserialize(std::move(mbuffer));
  // ...
```

### Examples

The communication between nodes happens in a using standardized *network packets*
that contain data, commands, or parameters. Here we give some examples of these
packets.

#### Adapter to Reconstructor

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

#### Reconstructor to/from Visualizer

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

#### Other uses

* Plugin system: make a server that reads a SliceDataPacket, modifies it and sends it along the pipeline.
Only necessary change: slice data gets sent to plugin socket instead of RECAST, while plugin forwards it to RECAST
* Real-time alignment: e.g. send a control packet ‘rotation_axis’. This creates a slider in RECAST, and the reconstruction software gets notified when the user changes this. 
Can also be used for checkboxes (Gaussian pass, Paganin, ...), 
or drop down menus (changes FBP filter used, …)
* Multi-GPU FBP/FDK reconstruction distribute ProjectionDataPackets round robin, and
sum the resulting ‘SliceDataPacket’s at RECAST3D (using the member ‘additive’).


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

### Acquisition geometries

The fields of the acquisition geometry packets:

- `tomop.parallel_beam_geometry_packet`
- `tomop.parallel_vec_geometry_packet`
- `tomop.cone_beam_geometry_packet`
- `tomop.cone_vec_geometry_packet`

follow the conventions of the constructors of 3D Geometries of the ASTRA
Toolbox. See <http://www.astra-toolbox.com/docs/geom3d.html> for an overview.




## Usage

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
