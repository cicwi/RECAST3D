### Writing an adapter

Each scanner or data source, has different formats for storing projection data, specifying the (acquisition) geometry, and different requirements on data preprocessing.

An adapter generates and sends three types of standardized packets from the specific scanner data:

- *Scan settings*: `tomop.scan_settings_packet`
- *Geometry*: `geometry_specification_packet` for the volume geometry, and e.g. `tomop.parallel_beam_geometry_packet` for the acquisition geometry.
- *Projection data*: `tomop.projection_packet`

This gets sent (one way) to the projection server which is ‘scanner agnostic’. To support a different scanner (or microscope, beamline, dataset), you can write a straightforward data adapter in the form of simple Python script. Example adapters can be found in the `examples` directory of TomoPackets.

To implement an adapter, we have to send the three types of packets to a listening
reconstructor. To send these packets, we can use a `tomop.publisher` object. An example is provided below::

```python
{! pages/users/zero_adapter.py !}
```

