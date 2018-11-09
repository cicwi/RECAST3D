# TomoPackets

A library for real-time tomography pipelines based on ZeroMQ.

This library defines a protocol for sending messages between the different
components (scanners, reconstruction nodes, visualization workstations) for
real-time tomographic reconstruction.

## Installation

### Dependencies

- _Boost_ (required). We use Boost.Hana for struct introspection. CMake tries to
  find Boost with `find_package`.
- _pybind11_ (submodule). Supplied as a git submodule. Run `git submodule --init
  --remote --recursive` before following the language specific instructions
  below.
- _ZeroMQ_ (optional).
- _cppzmq_ (optional).

If optional dependencies are not found using CMake `find_package`, they will be
cloned as a git submodule and built from source.

### C++

The easiest way to interface with tomopackets is using CMake. We define a
`INTERFACE` target `tomop` that can be used as follows. We assume the
TomoPackets repository is in `ext/tomop`. For example, by managing it as a Git
submodule:

```bash
git submodule add https://www.github.com/cicwi/tomopackets ext/tomop
git submodule update --init
```

Next, add the repository as a subdirectory and link your targets against
`tomop`.

```cmake
add_subdirectory("ext/tomop")
target_link_libraries(your_program tomop)
```

### Python

To install the library locally, simply run:

```bash
pip install -e .
```

## Examples

The following example sets up a reconstruction server for on-demand slice
reconstruction:

```python
import tomop
import numpy as np


def callback(orientation, slice_id):
    print("callback called")
    print(orientation)
    return [4, 4], np.array([0, 255, 0, 255, 255, 0, 255, 0, 255,
                             0, 0, 255, 255, 0, 0,
                             255], dtype='float32')

serv = tomop.server("hi")

vdp = tomop.volume_data_packet(
    serv.scene_id(),
    np.array([2, 2, 2], dtype='int32').tolist(),
    np.array([0, 255, 128, 255, 255, 128, 255, 0], dtype='float32'))

serv.send(vdp)

serv.set_callback(callback)
serv.serve()
```

## Authors and contributors

Bulk is developed at Centrum Wiskunde & Informatica (CWI) in Amsterdam by:

* Jan-Willem Buurlage (@jwbuurlage)

Also thanks to:

* Willem Jan Palenstijn (@wjp)

## Contributing

We welcome contributions. Please submit pull requests against the develop
branch.

If you have any issues, questions, or remarks, then please open an issue on
GitHub.

## Please Cite Us

If you have used TomoPackets for a scientific publication, we would appreciate
citations to the following paper:

[Real-time quasi-3D tomographic reconstruction. JW Buurlage H Kohr WJ Palenstijn
KJ Batenburg. Measurement Science and Technology
(2018)](https://doi.org/10.1088/1361-6501/aab754)

## License

This project is licensed under the GPL. See `LICENSE.md` for details.

