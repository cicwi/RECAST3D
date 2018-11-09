# Changelog

All notable changes to TomoPackets will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic
Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Add `ScanSettingsPacket` for setting number of darks/flats.

### Fixed
- Fix deserialization to include descriptor

## 1.0.0-rc1 - 2018-11-08

Candidate for initial release. Changes are compared to previous development
versions.

### Changed

- Constructor of `SliceDataPacket` now has the additive `bool` flag as final parameter.

[Unreleased]: https://github.com/cicwi/tomopackets/compare/v1.0.0-rc1...HEAD
