Slicevis
========

Dependencies
============

* `dear imgui,`, `catch`, `zmq` and `cppzmq` provided as submodules
* `OpenGL`
* `glfw3`
* `glm`

Installation
============

```bash
git submodule update --init --remote
```

Build ZMQ:

```
cd ext/libzmq/
mkdir build
cd build
cmake ..
make -j8
```

Authors
=======

Slicevis is developed by the Computational Imaging group at CWI.

- Jan-Willem Buurlage
