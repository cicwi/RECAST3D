# Releasing a new version


1. Update version number in the following places:
    - `recast3d/conda/meta.yaml`
    - `slicerecon/conda/meta.yaml`
    - `slicerecon/setup.py`
    - `tomopackets/conda/meta.yaml`
    - `tomopackets/setup.py`
1. Edit `CHANGELOG.md`:
    1. Change `[Unreleased]` to `[X.Y.Z] - YYYY-MM-DD`
    1. Add compare link at the bottom of the document
    1. Use commit message `Release X.Y.Z`.
1. Build conda packages:
    1. Run `conda build .` in `{recast3d,slicerecon,tomopackets}/conda/`
    1. Upload to the `cicwi` channel of `anaconda.org`
1. Merge `develop` into `master`.
1. Make a new release on GitHub and copy the the content of the old `[Unreleased]` section of the CHANGELOG into the description.
