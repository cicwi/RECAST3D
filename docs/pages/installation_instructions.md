# Installation

The CI CWI group develops a set of libraries and software for real-time tomographic reconstruction.
The main software packages are:

- _RECAST3D_. The visualization software. This typically runs at a workstation,
  and is controlled by a user looking at live reconstructions of arbitrarily
  oriented slices.
- _SliceRecon_. The reconstruction software library. This runs on a computer
  with a GPU (it can be the same as the workstation running RECAST3D). It
  receives the projection data, and performs slice reconstructions.
- _TomoPackets_. The glue between the different components. This library
  defines a protocol for sending messages between scanners, reconstruction
  nodes, and visualization workstations.

Depending on the data source, an _adapter_ is needed that converts the
acquisition metadata and projection data into a common format defined by
TomoPackets. This common format that is used as input to SliceRecon.

## (Recommended) Installation using Conda

The RECAST3D reconstruction stack can be installed using:

```bash
conda install -c cicwi -c astra-toolbox/label/dev recast3d tomopackets slicerecon
```

Example scripts such as adapters and plugins, can be found in the [GitHub repository](https://github.com/cicwi/RECAST3D/). To test the installation, follow [running the stack](users/live_scan.md).

!!! info
    The software is developed for Linux. SliceRecon requires a CUDA capable GPU.


## (Advanced) Manual installation

Here, we will describe in detail how to manually build all the different
components of the live reconstruction software stack.

!!! warning
    If you run into unexpected problems while following this guide, [please let us know](https://github.com/cicwi/RECAST3D/issues)!.

### 0. (optional) Setting up a fresh _Conda_ (or _VirtualEnv_) environment

Although it is not strictly necessary to use any Python component, it is by far
the easiest way to extend and customize the software. We would suggest making a
new environment in which the Python bindings to our libraries are installed. For
example:

```bash
conda create -n live python=3.7
conda activate live
```

if you choose to do this, make and activate the environment before following the
rest of these instructions.

### 1. Building and installing ASTRA


#### Recommended: Using conda

!!! info
    Currently (March 2020), the required files are only part of the development package of ASTRA. This can be installed using:

    ```
    conda install -c astra-toolbox/label/dev astra-toolbox
    ```


Install the ASTRA toolbox into your conda environment using:

```bash
conda install -c astra-toolbox astra-toolbox
```

ASTRA also includes headers and configuration files as part of its conda package.

#### Alternative: From source

!!! warning
    The software requires at least version v1.9 of the ASTRA toolbox,
    available from [the ASTRA repository on github](https://github.com/astra-toolbox/astra-toolbox).

We have to install the [ASTRA toolbox](http://www.astra-toolbox.com/). We
need ASTRA with CUDA support, and install it in such a way so that `pkg-config`
is able to find the library. See the ASTRA documentation for details on installing ASTRA as a C++ library.


#### Setup `pkgconfig`
The rest of this guide assumes that pkgconfig can find the ASTRA configuration, for this you might have to run something along the lines of:

```bash
# when installed using conda
export PKG_CONFIG_PATH=$CONDA_PREFIX/lib/pkgconfig/:$PKG_CONFIG_PATH

# when installed from source
export PKG_CONFIG_PATH=$ASTRA_SRC_DIR/build/linux:$PKG_CONFIG_PATH
```

### 2. Installing prerequisites

Next, make sure you have the necessary OpenGL development headers and libraries and other dependencies
installed. This highly depends on your operating system and/or distribution. A non-exhaustive lists of packages that might be required on Fedora 31:

```bash
dnf install python-devel boost-devel libXinerama-devel fftw-devel
dnf groupinstall "X Software Development"
dnf groupinstall "Development Tools"
```

Optionally, preinstall the submodule dependencies to speed up the build. For example:

```bash
dnf install python-devel eigen3-devel boost-devel libXinerama-devel glm-devel
```

If our build script cannot find these dependencies, it
will build them from source.

### 3. Cloning the repository

```cpp
git clone https://www.github.com/cicwi/RECAST3D recast3d-stack
cd recast3d-stack
git submodule update --init --recursive
```

### 4. Building the software

#### TomoPackets

```bash
cd tomopackets
mkdir build && cd build
cmake .. && make
```

When developing your own data adapter (or using an existing one), it is easiest
to use the Python bindings to the TomoPackets communication library. To install
it into your Python environment, run the following:

```bash
cd ..
pip install --user cmake_setuptools
pip install -e .
```

You can now develop and run TomoPackets extensions such as data adapters.

#### SliceRecon


Next, we install SliceRecon.

```bash
cd ../slicerecon
mkdir build && cd build
cmake .. && make
```

This will create a binary `slicerecon_server`.

Next, we install the Python bindings.

```bash
cd ../python/
pip install -e .
```

#### RECAST3D

To build RECAST3D, run:

```bash
cd ../../recast3d
mkdir build && cd build
cmake .. && make
```

This will create a binary `recast3d`. Test if you can run RECAST3D:
```bash
./recast3d
```
This should open an empty white window with a menubar. If you run into any issues, [please let us know](https://github.com/cicwi/RECAST3D/issues)!

### 5. Testing the installation

To test if the SliceRecon software is installed correctly, try the following steps.

1. _Start up a RECAST3D instance_. Change directories to the `recast3d/build` directory we made before, and run:

        ./recast3d

2. _Start up a SliceRecon server_, that connects to RECAST3D and listens for
   incoming (projection) data. Change directories to `slicerecon/build`, and run:

        ./slicerecon_server --slice-size 512 --preview-size 256
   Switching windows to the RECAST3D software, you should see a new scene with
   empty slices.

3. _Pushing data into SliceRecon_. The server is unable to reconstruct anything
   interesting, because it has received no data yet. To push some example data
   into SliceRecon, we run one of the adapter examples. For example, relative to the `recast3d_stack` directory:

        cd examples/adapters
        python zero_adapter.py

   This will send data containing zeros to the SliceRecon server.
