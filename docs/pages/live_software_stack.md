# Installing the CI CWI live reconstruction software stack

For realizing live reconstruction of tomographic scans, the CI CWI group
develops a set of libraries and software that can work together. The main
software are:

- _RECAST3D_. The visualization software. This typically runs at a workstation,
  and is controlled by a user looking at live reconstructions of arbitrarily
  oriented slices.
- _[SliceRecon]_. The reconstruction software library. This runs on a computer
  with a GPU (it can be the same as the workstation running RECAST3D). It
  receives the projection data, and performs slice reconstructions.
- _[TomoPackets]_. The glue between the different components. This library
  defines a protocol for sending messages between scanners, reconstruction
  nodes, and visualization workstations.

Depending on the data source, an _adapter_ is needed that converts the
acquisition metadata and projection data into a common format defined by
TomoPackets, and that can be sent as input to SliceRecon.

Further reading:

- [TomoPackets#Pipeline](https://cicwi.github.io/TomoPackets/overview.html#pipeline)
  for an overview of the pipeline.
- [TomoPackets#Usage](https://cicwi.github.io/TomoPackets/usage.html#writing-an-adapter)
  for documentation on writing an adapter.

[TomoPackets]: https://www.github.com/cicwi/TomoPackets
[SliceRecon]: https://www.github.com/cicwi/SliceRecon

## Detailed installation instructions

For convenience, we will describe in detail how to install all the different
components of the live reconstruction software stack.

!!! warning
    If you have trouble following this guide, please first see the documentation
    of each individual package for the most up-to-date installation instructions.
    If that still does not help, [please let us know](https://github.com/cicwi/RECAST3D/issues)!.

### 0. (optional) Setting up a fresh _Conda_ or _VirtualEnv_ environment

Although it is not strictly necessary to use any Python component, it is by far
the easiest way to extend and customize the software. We would suggest making a
new environment in which the Python bindings to our libraries are installed. For
example:

```bash
conda create -n live python=3.6
conda activate live
# ... or `source activate live` for old conda versions
```

if you choose to do this, make and activate the environment before following the
rest of these instructions.

### 1. Installing RECAST3D

First, we will install the RECAST3D visualization software.

```bash
# make a directory to store the packages
mkdir projects/live_reconstruction
cd projects/live_reconstruction
# clone the repository
git clone https://github.com/cicwi/RECAST3D.git/
cd RECAST3D
```

Next, make sure you have GLM and the OpenGL development headers and libraries
installed. On Fedora, this would require the `glm-devel` and for example
`mesa-libGL-devel` packages. Optionally, preinstall the submodule dependencies
to speed up the build. If our build script cannot find these dependencies, it
will build them from source.

To build RECAST3D, run e.g.:
```bash
git submodule update --remote --init --recursive
mkdir build
cd build
cmake ..
make -j8
```

Finally, test if you can run RECAST3D:
```bash
./recast3d
```
This should open an empty white window with a menubar.
If you run into any issues, [please let us know](https://github.com/cicwi/RECAST3D/issues)!

### 2. Installing SliceRecon

Next, we install SliceRecon.

```bash
# go back to the root directory:
cd projects/live_reconstruction
# clone the repository
git clone https://github.com/cicwi/SliceRecon.git/
cd SliceRecon
```

Next, we need to install the dependencies.

!!! warning
    The software requires a development version of the ASTRA toolbox,
    available from [the ASTRA repository on github](https://github.com/astra-toolbox/astra-toolbox).
    The required functionality will be released in ASTRA v1.9.

First, we have to install the [ASTRA toolbox](http://www.astra-toolbox.com/). We
need ASTRA with CUDA support, and install it in such a way so that `pkg-config`
is able to find the library. See the ASTRA documentation for details on installing ASTRA as a C++ library.

!!! note
    On CI CWI workstations, you can use the module system: `module load libastra/git`.

Again, preinstalling the submodule dependencies will speed up the build.

```bash
git submodule update --remote --init --recursive
mkdir build
cd build
cmake ..
make -j8
```

#### Installing the SliceRecon Python bindings

Next, we install the Python bindings.

```bash
cd ../python/ # relative to the build directory
pip install -e .
```

#### Testing the installation

To test if the SliceRecon software is installed correctly, try the following steps.

1. _Start up a RECAST3D instance_. Change directories to the build folder of the
   RECAST3D directory we made before, and run:

        ./recast3d

2. _Start up a SliceRecon server_, that connects to RECAST3D and listens for
   incoming (projection) data. Change directories to the build folder of the
   SliceRecon repository, and run:

        ./example/slicerecon_server --slice-size 512 --preview-size 256
   Switching windows to the RECAST3D software, you should see a new scene with
   black slices.

3. _Pushing data into SliceRecon_. The server is unable to reconstruct anything
   interesting, because it has received no data yet. To push some example data
   into SliceRecon, run the following (from the same folder as before):

        ./example/slicerecon_push_tomos --parallel
   This will send very poor quality and low-resolution projection data
   of a Shepp--Logan phantom to the server, but you should be able to
   see some reconstructions in the RECAST3D software. Expect to wait a
   while before seeing anything on screen.

### 3. (optional) Installing TomoPackets

When developing your own data adapter (or using an existing one), it is easiest
to use the Python bindings to the TomoPackets communication library. To install
it into your Python environment, run the following:

```bash
# go back to the root directory:
cd projects/live_reconstruction
# clone the repository
git clone https://github.com/cicwi/TomoPackets.git/
cd TomoPackets
```

To install the library, simply run:
```
git submodule update --init --remote
pip install -e .
```
You can now develop and run TomoPackets data adapters.
