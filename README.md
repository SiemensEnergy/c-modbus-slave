# C Modbus Slave

[![License](https://img.shields.io/github/license/SiemensEnergy/c-modbus-slave.svg)](https://github.com/SiemensEnergy/c-modbus-slave/blob/master/LICENSE)
[![Release](https://img.shields.io/github/v/release/SiemensEnergy/c-modbus-slave)](https://github.com/SiemensEnergy/c-modbus-slave/releases)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![MISRA C](https://img.shields.io/badge/MISRA-C%3A2012-brightgreen.svg)](https://misra.org.uk/)
[![Stars](https://img.shields.io/github/stars/SiemensEnergy/c-modbus-slave?style=social)](https://github.com/SiemensEnergy/c-modbus-slave/stargazers)

Lightweight [Modbus](https://en.wikipedia.org/wiki/Modbus) slave stack for embedded C applications

## Features

- **Zero dynamic allocations** - Can be used without any dynamic allocations
- **Transport agnostic** - Works with RS485/RS232 serial and Ethernet
- **Flexible data access** - Direct pointers, callbacks, or constant values
- **Compact footprint** - Minimal RAM and flash usage for embedded systems
- **Fast CRC calculation** - Uses lookup table for efficient CRC-16 computation (512 bytes ROM)
- **Standards compliant** - Implements Modbus specification accurately
- **Thread-safe** - No global state, multiple instances supported

## Getting Started

### Prerequisites

- C11 compatible compiler
- Basic knowledge of Modbus protocol ([Wikipedia](https://en.wikipedia.org/wiki/Modbus), [Specs](https://www.modbus.org/modbus-specifications))

### Quick Example

Here's a minimal example to get you started:

```c
#include "mbinst.h"

/* 1. Define your data */
static uint16_t s_temperature = 250;

/* 2. Create register map (must be in ascending address order) */
static const struct mbreg_desc_s s_holding_regs[] = {
    {
        .address=0x00,
        .type=MRTYPE_U16,
        .access=MRACC_RW_PTR,
        .read={.pu16=&s_temperature},
        .write={.pu16=&s_temperature}
    }
};

/* 3. Create instance */
static struct mbinst_s s_inst = {
    .hold_regs=s_holding_regs,
    .n_hold_regs=sizeof s_holding_regs / sizeof s_holding_regs[0]
};

/* 4. Initialize internal instance state */
mbinst_init(&s_inst);

/* 5. Handle incoming requests */
uint8_t req[MBADU_SIZE_MAX];
size_t req_len = fetch_request(req); /* Fetch data from your transport layer (see examples/**) */

uint8_t resp[MBADU_SIZE_MAX];
size_t resp_len = mbadu_handle_req(&s_inst, req, req_len, resp);
if (resp_len) {
    send_response(resp, resp_len); /* Send response via your transport layer (see examples/**) */
}
```

> **Tip:** Check the `examples/` directory for complete working examples with different transport layers (Serial, Ethernet).

## Documentation

- **[Building & Setup](docs/building.md)** - Build instructions and file dependencies
- **[API Reference](docs/api-reference.md)** - API documentation and function signatures
- **[Examples](docs/examples.md)** - Usage examples for different scenarios
- **[Testing](docs/testing.md)** - Unit testing and validation tools
- **[Glossary](docs/glossary.md)** - Abbreviations and technical terms

## Code Quality

This project follows **MISRA C:2012** coding standards with a few documented exceptions where necessary for functionality or compatibility. The codebase emphasizes:

- Static analysis compliance for safety-critical applications
- Predictable behavior and reduced complexity
- Enhanced portability across embedded platforms
- Clear documentation of any deviations from the standard

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
