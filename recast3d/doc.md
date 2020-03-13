# TODO:
- [x] Should be able to update plot data
- [x] The scene should have metadata such as size and scale
    - [x] change scene to make pixels flexible, a scene should have a size
    - [x] add size information to packets
- [ ] Allow choosing colorscheme
    - [x] Support for colorschemes
    - [ ] UI dropdown menu
- [x] Add support for 3D
- [ ] Add support for distributed image information (i.e. partial image updates)

- [ ] Add support for low resolution 3d information, instead of checkboard slices
- [ ] Send slice changed packet, set old slice inactive (can't select anymore, gets removed when new data comes in)
- [ ] When plotting distributed, only 'gather' slices, not entire image.
