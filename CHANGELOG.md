# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Added file record descriptors and supporting functions
- Handle function code 0x14 and 0x15 for file record access

### Fixed

- Validate protocol length in mbadu_tcp

## [1.4.0] - 2025-11-09

### Added

- Function 0x16 (Mask write register)
- Add support for writing to partial registers with function write access

## [1.3.0] - 2025-10-20

### Changed

- Coil read and write callbacks now operate on `int` instead of `uint8_t`
- Ensure mostly MISRA compliance with a few exceptions

## [1.2.1] - 2025-09-17

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
