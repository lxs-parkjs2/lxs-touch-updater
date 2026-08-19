# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [0.0.2] - 2026-08-19

### Changed
- Makefile: use implicit variables (`?=`) to support cross-compilation
- Makefile: remove `-static-libstdc++` and `-static-libgcc` flags

### Added
- CHANGELOG.md

## [0.0.1] - 2026-07-15

### Added
- Initial release of LXS touch controller firmware updater for ChromeOS
- Support for firmware version query (`--get_current_version`)
- Support for firmware update (`--update`)
- HID raw interface communication via `/dev/hidraw*`
- DFU mode switching and flash write support (4KB and normal mode)
