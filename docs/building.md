# Building & Setup

This document provides information on building and setting up your project.

## File Dependencies

Files marked **X** must always be compiled, regardless of transport protocol.

|       | File          | Note                |
| ----- | ------------- | ------------------- |
| **X** | endian.c      |                     |
|       | mbadu.c       | _Serial RTU only_   |
|       | mbadu_ascii.c | _Serial ASCII only_ |
|       | mbadu_tcp.c   | _TCP/IP only_       |
| **X** | mbcoil.c      |                     |
| **X** | mbcrc.c       |                     |
| **X** | mbfile.c      |                     |
| **X** | mbfn_coils.c  |                     |
| **X** | mbfn_diag.c   |                     |
| **X** | mbfn_files.c  |                     |
| **X** | mbfn_regs.c   |                     |
| **X** | mbfn_serial.c |                     |
| **X** | mbinst.c      |                     |
| **X** | mbpdu.c       |                     |
| **X** | mbreg.c       |                     |
|       | mbsupp.c      | _If needed_         |
|       | mbtest.c      | _Unit testing only_ |

## Compiler Requirements

- **C11 compatible compiler**
