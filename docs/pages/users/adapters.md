### Writing an adapter

Each ‘scanner’ has different data formats, geometry specification, and preprocessing requirements

An adapter generates three standardized packets from the specific scanner data:

- Scan settings
- Geometry
- Projection data

This gets sent (one way) to the ‘projection server’, which is ‘scanner agnostic’. To support a different scanner (microscope, beamline, dataset), just make a simple adapter (Python script). Example adapters can be found in the `examples` directories.

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

