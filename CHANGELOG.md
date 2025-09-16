# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Added support for register block access of 8-bit values

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
