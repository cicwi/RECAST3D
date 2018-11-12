# SliceRecon

A library for on-demand tomographic reconstructions of arbitrary slices.

This project contains efficient reconstruction code for reconstructing
arbitrarily oriented slices through a 3D volume. It defines servers that are
able to communicate to data sources (using [TomoPackets]) and visualization
software (such as [RECAST3D]).

## Installation
Dependencies
- ASTRA + CUDA (required). We use the ASTRA toolbox for reconstructing the
  slices.
- Eigen (submodule).
- TomoPackets (submodule)
- Tomos (submodule)
- ZeroMQ / cppzmq (optional)

If optional dependencies are not found using CMake's `find_package`, they will
be cloned as a git submodule and built from source.

### C++

Building the reference server, and example data adapters:

```
module load [...] # optional: load in preinstalled dependencies
git submodule update --init --remote --recursive
mkdir build
cd build
cmake ..
make -j8
```

### Python

The supplied Python bindings are intended for plugin development. Install using:

```bash
cd python
pip install -e .
```

## Examples

Developing a plugin using Python:

```python
import numpy as np
import slicerecon


def callback(shape, xs, idx):
xs = np.array(xs).reshape(shape)

    print("callback called", shape)
    xs[xs <= 3.0] = 0.0
    xs[xs > 3.0] = 10.0

    return [shape, xs.ravel().tolist()]


p = slicerecon.plugin("tcp://*:5652", "tcp://localhost:5555")
p.set_slice_callback(callback)

p.listen()
```

Host a projection and visualization server:

```cpp
// ...
// 1. setup reconstructor
auto recon = std::make_unique<slicerecon::reconstructor>(params);

// 2. listen to projection stream
// projection callback, push to projection stream
// all raw data
auto proj = slicerecon::projection_server(host, port, *recon,
                                          use_reqrep ? ZMQ_REP : ZMQ_PULL);
proj.serve();

// 3. connect with (recast3d) visualization server
auto viz = slicerecon::visualization_server(
    "slicerecon test", "tcp://"s + recast_host + ":5555"s,
    "tcp://"s + recast_host + ":5556"s);
viz.set_slice_callback(
    [&](auto x, auto idx) { return recon->reconstruct_slice(x); });

recon->add_listener(&viz);
viz.serve();
// ...
```

## Authors and contributors
SliceRecon is developed at Centrum Wiskunde & Informatica (CWI) in Amsterdam by:

- Jan-Willem Buurlage (@jwbuurlage)

Also thanks to:

- Willem Jan Palenstijn (@wjp)

## Contributing

We welcome contributions. Please submit pull requests against the `develop` branch.

If you have any issues, questions, or remarks, then please open an issue on GitHub.

## Please Cite Us

If you have used SliceRecon for a scientific publication, we would appreciate
citations to the following paper:

[Real-time quasi-3D tomographic reconstruction. JW Buurlage, H Kohr, WJ
Palenstijn, KJ Batenburg. Measurement Science and Technology
(2018)](https://doi.org/10.1088/1361-6501/aab754)

## License

This project is licensed under the GPL. See `LICENSE.md` for details.

[TomoPackets]: https://www.github.com/cicwi/tomopackets
[RECAST3D]: https://www.github.com/cicwi/RECAST3D
