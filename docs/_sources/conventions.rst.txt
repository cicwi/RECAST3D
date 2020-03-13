===========
Conventions
===========

Multi-dimensional arrays
------------------------

- Volume data is stored in x-y-z order (from major to minor).
- Projection data is stored in row-column order (from major to minor).

Slice orientation
-----------------

We need a convention for representing the orientation of a slice. The
orientation is inside *volume space* and is completely independent from the
number of pixels (i.e. the 'size' of an individual pixel is implied by the
bounding square of a slice). We represent the orientation as 9 real numbers
:math:`(a, b, \ldots, i)` so that:

.. math::
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

Where the vector :math:`\vec{x}_s = (x_s, y_s)` lives inside a slice (i.e. the
*normalized* pixel coordinates of a slice, in the interval :math:`[0, 1]`), and where
:math:`\vec{x}_w = (x_w, y_w, z_w)` lives inside the volume geometry at the correct
place. The pixel coordinates of a slice have the following convention:

.. math::
  \begin{equation}
  \begin{pmatrix}
  (0, m) & \cdots & (n, m) \\
  \vdots & \ddots & \vdots \\
  (0, 0) & \cdots & (n, 0)
  \end{pmatrix}
  \end{equation}

i.e. we start counting from the bottom-left and use a standard cartesian
:math:`xy` convention.

Using this convention, the vector :math:`\vec{b} = (g, h, i)` is the base point
of the slice in world space (i.e. the world coordinates of the bottom left point
of a slice). :math:`\vec{x} = (a,b,c)` is the direction in world space
corresponding to the :math:`x` direction of the slice, and :math:`\vec{y} = (d,
e, f)` corresponds to the :math:`y` direction.

Acquisition geometries
----------------------

The fields of the acquisition geometry packets:

- :class:`tomop.parallel_beam_geometry_packet`
- :class:`tomop.parallel_vec_geometry_packet`
- :class:`tomop.cone_beam_geometry_packet`
- :class:`tomop.cone_vec_geometry_packet`

follow the conventions of the constructors of 3D Geometries of the ASTRA
Toolbox. See http://www.astra-toolbox.com/docs/geom3d.html for an overview.
