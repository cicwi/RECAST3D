# Changelog

All notable changes to TomoPackets will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic
Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Add re-export of `scan_settings_packet` to the `tomop` Python library.

## [1.0.0-rc2] - 2018-11-12

### Added
- Add `ScanSettingsPacket` for setting number of darks/flats.
- Add explicit values to descriptors

### Fixed
- Fix deserialization to include descriptor
- Use mutable accessors in `fill`

## 1.0.0-rc1 - 2018-11-08

Candidate for initial release. Changes are compared to previous development
versions.

### Changed

- Constructor of `SliceDataPacket` now has the additive `bool` flag as final parameter.

[Unreleased]: https://github.com/cicwi/tomopackets/compare/v1.0.0-rc2...develop
[1.0.0-rc2]: https://github.com/cicwi/tomopackets/compare/v1.0.0-rc1...v1.0.0-rc2
