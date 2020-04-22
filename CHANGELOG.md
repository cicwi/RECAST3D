# Changelog

All notable changes to the RECAST3D stack will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased] - YYYY-MM-DD

### Added
#### SliceRecon
- Expose reconstructor to the Python bindings
- Allow setting a custom filter using an array instead of a file
- Add `mute` functionality to logger

#### RECAST3D
- Use and distribute [FiraCode](https://github.com/tonsky/FiraCode/) font

### Fixed
#### SliceRecon
- Fix slice reconstruction scaling being dependent on preview size
- Parallel beam geometries now correctly follow the ASTRA conventions for detector positioning in parallel vector geometries.
- Orientations are now normalized as intended: the coordinate system in which slices are defined are scaled such that the 3D object volume is in $[-1, 1]^3$.

## [1.1.0] - 2020-27-03

### Added
#### RECAST3D
- Add support for trackers, benchmarks, and parameter control for upstream
  components
- Add experimental transparent reconstruction mode (this makes air on slices see-through)
- Add feature to set permanently fixed slices for analysis with middle mouse button

#### TomoPackets
- Add re-export of `scan_settings_packet` to the `tomop` Python library.
- Add named arguments to `tomop` class constructors.
- Add `already_linear` flag to `ScanSettingsPacket`.
- Add `Parameter{Bool,Float,Enum}Packet`, `TrackerPacket` and `BenchmarkPacket`.
- Add support for `std::vector<std::string>` fields.

#### SliceRecon
- Add `continuous` flag, for reconstructions happening more often than one rotation (#4).
- Add `already_linear` flag, for already linearized data.
- Add support for skipping flat fielding step.
- Add Gaussian filter for optional noise suppression
- Add support for Paganin filtering
- Add support for manually choosing tilt axis for parallel beam geometries
- Add `bench` flag for (optional) benchmarking support

### Fixed
#### RECAST3D
- Fix scaling of 3D volume preview in reconstruction

#### SliceRecon
- Fix application of FDK weighting.
- Fix possible simultaneous access to a plugin socket
- Fix uploads not triggering when `group_size` did not divide `proj_count` (#9)
- Change `plugin::listen` to run on the main thread
- Change default value for flat field to `1`, darks and flats are now optional (#6).
- Use FFTW3 as the FFT backend (instead of an unsupported Eigen module).
- More modular system for processing projections

### Changed
#### RECAST3D
- Initial scaling is now based on first received _nonzero_ data

## [1.0.0-rc.1] - 2018-11-13

### Added
- Add documentation
- Improved build scripts 
- Use TomoPackets v1.0.0

### Removed
- Disable 2D scenes
- Disable movie scenes
- Disable dataset loading

## 0.1.0 - 2018-06-21

- Initial release.

[Unreleased]: https://github.com/cicwi/RECAST3D/compare/v1.1.0...develop
[1.1.0]: https://github.com/cicwi/RECAST3D/compare/v1.0.0-rc.1...v1.1.0
[1.0.0-rc.1]: https://github.com/cicwi/RECAST3D/compare/v0.1.0...v1.0.0-rc.1
