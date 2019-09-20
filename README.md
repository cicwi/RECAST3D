RECAST3D
========

REConstruction of Arbitrary Slices in Tomography

![](https://raw.githubusercontent.com/cicwi/RECAST3D/develop/docs/preview_usage.gif)

This visualization software for tomography, is built for use in a distributed,
real-time live reconstruction pipeline. It uses [TomoPackets] as its protocol
for requesting and receiving slice reconstructions, and other data.

See also:

- [TomoPackets]
- [SliceRecon]

Dependencies
============

* `OpenGL` (required) development libraries and headers
* `glm` (required)
* `GLFW` (submodule)
* `Assimp` (submodule)
* `ImGui` (submodule)
* `ZeroMQ` (submodule)
* `cppzmq` (submodule)
* `Eigen` (submodule)
* `TomoPackets` (submodule)

Installation
============

Build RECAST3D:
---------------

After installing the dependencies, run:

```
git submodule update --remote --init --recursive
mkdir build
cd build
cmake ..
make -j8
```

The RECAST3D binary can then be found in the `build` directory.

Using conda:
------------

If you have conda, then RECAST3D can be installed with:

```
conda install -c cicwi recast3d
```

RECAST3D can then be started by running:

``` bash
recast3d
```

Authors
=======

RECAST3D is developed by the Computational Imaging group at CWI. Main author:

- Jan-Willem Buurlage

Also thanks to:

- Holger Kohr
- Willem Jan Palenstijn

## Contributing

We welcome contributions. Please submit pull requests against the develop
branch.

If you have any issues, questions, or remarks, then please open an issue on
GitHub.

## Please Cite Us

If you have used RECAST3D for a scientific publication, we would appreciate
citations to the following paper:

[Real-time quasi-3D tomographic reconstruction. JW Buurlage, H Kohr, WJ
Palenstijn, KJ Batenburg. Measurement Science and Technology
(2018)](https://doi.org/10.1088/1361-6501/aab754)

## License

This project is licensed under the GPL. See `LICENSE.md` for details.

[TomoPackets]: https://www.github.com/cicwi/TomoPackets
[SliceRecon]: https://www.github.com/cicwi/SliceRecon

## Acknowledgments

- [Iosevka font](https://github.com/be5invis/Iosevka). Copyright (c) 2015-2017
  Belleve Invis (belleve@typeof.net).
