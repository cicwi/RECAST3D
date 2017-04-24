Slicevis
========

Dependencies
============

* `dear imgui,`, `catch`, `zmq` and `cppzmq` provided as submodules
* `OpenGL`
* `glfw3`

Install
=======

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
