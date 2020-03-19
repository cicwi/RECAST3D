# Running the stack

## Starting RECAST3D

First, we have to start up RECAST3D. This either happens from the application launcher of your OS, or using a Linux terminal:

```bash
# Inside a conda environment with RECAST3D intstalled
conda activate [your_environment]
recast3d

# ... or inside the 'build' directory for a manual install
cd recast3d/build
./recast3d
```

## Starting the SliceRecon reconstruction server

After RECAST3D is running, we can start a reconstruction server. By default, we can run a SliceRecon server:

```bash
cd slicerecon/build
./slicerecon_server [options]
```

For example, with `[options]` we can set the slice resolution, preview resolution, and so on. For a full list of options, run with `--help`:

```bash
./slicerecon_server --help
```

When a server is started, it connects with RECAST3D and tells it to create a new scene. After starting the server, the RECAST3D window should show a new scene with three inactive slices.

## Pushing data into the SliceRecon server using an adapter

The server is now waiting until (projection) data is pushed to it. For example,
we can push prerecorded data from the FleX-ray lab:

```bash
cd examples/adapters
python slicerecon_push_flexray.py [path]
```

## (Optional) plugins

To use or test (Python) plugins, we run the reconstruction server with `--pyplugin`.

```bash
./slicerecon_server --pyplugin [other options]
```

and start our plugin:

```bash
python plugin.py
```

After the plugin is started, we can push data to SliceRecon using any adapter.

To chain multiple plugins, simply change the outgoing host/port of a plugin to the incoming host/port of another. The final plugin should send the final processed slice data to RECAST3D using its host/port.

## Custom ports

The setup above works if you use the default ports, and when all components are run on `localhost`. The default ports are as follows:

- `5555`: RECAST3D `REQ/REP` server
- `5556`: RECAST3D `PUB/SUB` server
- `5558`: SliceRecon server
- `5652`: Python based plugin

These ports, as well as the host for RECAST3D or the (first) plugin, can be changed using flags to the SliceRecon server.
