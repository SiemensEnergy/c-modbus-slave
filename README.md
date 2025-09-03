# C Modbus Slave

Lightweight [Modbus](https://en.wikipedia.org/wiki/Modbus) slave implementation for embedded C applications.

## Features

- **Zero dynamic allocations** - Can be used without any dynamic allocations
- **Transport agnostic** - Works with RS485/RS232 serial and Ethernet
- **Flexible data access** - Direct pointers, callbacks, or constant values
- **Compact footprint** - Minimal RAM and flash usage for embedded systems
- **Fast CRC calculation** - Uses lookup table for efficient CRC-16 computation (512 bytes ROM)
- **Standards compliant** - Implements Modbus specification accurately
- **Thread-safe** - No global state, multiple instances supported

## Prerequisites

- C11 compatible compiler

## Quick Start

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
mbinst_init(&inst);

/* 5. Handle incoming requests */
uint8_t req[MBADU_SIZE_MAX];
size_t req_len = fetch_request(req); /* Fetch data from your transport layer (see examples/**) */

uint8_t resp[MBADU_SIZE_MAX];
size_t resp_len = mbadu_handle_req(&s_inst, req, req_len, resp);
if (resp_len) {
    send_response(resp, resp_len); /* Send response via your transport layer (see examples/**) */
}
```

## Documentation

- **[Building & Setup](docs/building.md)** - Build instructions and file dependencies
- **[API Reference](docs/api-reference.md)** - API documentation and function signatures
- **[Examples](docs/examples.md)** - Usage examples for different scenarios
- **[Testing](docs/testing.md)** - Unit testing and validation tools
- **[Glossary](docs/glossary.md)** - Abbreviations and technical terms

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
