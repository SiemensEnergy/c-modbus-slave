# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Read coil callback should now return an int instead of u8
- Ensure mostly MISRA compatibility with a few exceptions

## [1.2.1] - 2026-09-17

### Changed

- Improved reporting of device faults in coil handling
- Improved reporting of device faults in register handling

## [1.2.0] - 2025-09-16

### Added

- Added support for register block access of 8-bit values
- Improve runtime validation for incorrect register descriptor configuration

## [1.1.0] - 2025-09-03

### Changed

- Moved endian module to the main src directory to simplify building

## [1.0.1] - 2025-09-02

### Changed

- Don't increment comm event counter for diagnostic requests (Function code 0x08)
- Check CRC before slave address in ADU handling to monitor the overall health of the bus, not just this device
- Increment server message count before handling requests to act correct on communication reset
- Create send communication event for all processed requests

## [1.0.0] - 2025-09-02

### Added

- Initial open source release
