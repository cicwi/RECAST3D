.. highlight:: python

=====
Usage
=====

Writing an adapter
------------------

To implement an adapter, we have to send three types of packets to a listening
reconstructor. To send these packets, we can use a :class:`tomop.publisher`
object. An example is provided below::

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


Writing a reconstruction node
-----------------------------

For writing a simple reconstructor that responds to slice reconstruction
requests, you can use :class:`tomop.server` and set a callback. You can also
send other types of packets over the channel opened by this server. For example,
sending a :class:`tomop.volume_data_packet` enables a visualizer to show a 3D
preview::

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


