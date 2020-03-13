# Changelog

All notable changes to SliceRecon will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Add `continuous` flag, for reconstructions happening more often than one rotation (#4).
- Add `already_linear` flag, for already linearized data.
- Add support for skipping flat fielding step.
- Add Gaussian filter for optional noise suppression
- Add support for Paganin filtering
- Add support for manually choosing tilt axis for parallel beam geometries
- Add `bench` flag for (optional) benchmarking support

### Changed
- Change `plugin::listen` to run on the main thread
- Change default value for flat field to `1`, darks and flats are now optional (#6).
- Use FFTW3 as the FFT backend (instead of an unsupported Eigen module).
- More modular system for processing projections

### Fixed
- Fix application of FDK weighting.
- Fix possible simultaneous access to a plugin socket
- Fix uploads not triggering when `group_size` did not divide `proj_count` (#9)

## 1.0.0-rc.1

Initial release

[Unreleased]: https://github.com/cicwi/SliceRecon/compare/v1.0.0-rc.1...develop

