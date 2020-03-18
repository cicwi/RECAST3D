# Running the stack

## Starting the reconstruction server

Before running the reconstruction server, start up RECAST3D. To run the server:

```bash
  ./slicerecon_server [options]
```

The server is now waiting until (projection) data is pushed to it. For example,
we can push prerecorded FleX-ray data::

```bash
  python slicerecon_push_flexray.py [path]
```
