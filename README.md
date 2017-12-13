TomoPackets
-----------

## Installation

1. Get the submodules
    ```bash
    git submodule init
    git submodule update --remote
    ```
2. Build ZMQ (if not yet installed globally)
    ```bash
    cd ext/libzmq/
    ./autogen.sh
    mkdir build && cd build
    cmake ..
    make
    ```
3. (Optional) Build test server and python bindings
    ```bash
    # in project root directory
    mkdir build && cd build
    cmake ..
    make
    ```
4. (Optional) Install using pip
    ```bash
    pip install -e .
    ```
