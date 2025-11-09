# API Reference

## Core Functions

| Function                   | Description                                             |
| -------------------------- | ------------------------------------------------------- |
| `mbinst_init()`            | Initialize internal instance state with default values  |
| `mbadu_handle_req()`       | Process complete Modbus ADU _(Serial RS485/RS232)_      |
| `mbadu_ascii_handle_req()` | Process complete Modbus ADU _(Serial Ascii)_            |
| `mbadu_tcp_handle_req()`   | Process complete Modbus ADU _(TCP/IP)_                  |
| `mbpdu_handle_req()`       | Process Modbus PDU only _(for custom transport layers)_ |

### Function Signatures

```c
extern void mbinst_init(struct mbinst_s *inst);

extern size_t mbadu_handle_req(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    uint8_t *res);

extern size_t mbadu_ascii_handle_req(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    uint8_t *res);

extern size_t mbadu_tcp_handle_req(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    uint8_t *res);

extern size_t mbpdu_handle_req(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    uint8_t *res);
```

## Function Codes

Functions marked with **X** are handled by the library.
Additional function codes can be handled via the `mbinst_s::handle_fn_cb` callback.

Enum `enum mbfc_e` can be found in `mbdef.h`.

For additional information, see [~~Modbus Application Protocol~~](https://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf) [Modbus](https://www.modbus.org/).

|       | Value | Enum                           | Description                      | Note          |
| ----- | ----- | ------------------------------ | -------------------------------- | ------------- |
| **X** | 0x01  | **MBFC_READ_COILS**            | Read multiple coils              |               |
| **X** | 0x02  | **MBFC_READ_DISC_INPUTS**      | Read multiple discrete inputs    |               |
| **X** | 0x03  | **MBFC_READ_HOLDING_REGS**     | Read multiple holding registers  |               |
| **X** | 0x04  | **MBFC_READ_INPUT_REGS**       | Read multiple input registers    |               |
| **X** | 0x05  | **MBFC_WRITE_SINGLE_COIL**     | Write single coil                |               |
| **X** | 0x06  | **MBFC_WRITE_SINGLE_REG**      | Write single holding register    |               |
| **X** | 0x07  | **MBFC_READ_EXCEPTION_STATUS** | Read exception status            | _Serial only_ |
| **X** | 0x08  | **MBFC_DIAGNOSTICS**           | Diagnostics                      | _Serial only_ |
| **X** | 0x0B  | **MBFC_COMM_EVENT_COUNTER**    | Get comm event counter           | _Serial only_ |
| **X** | 0x0C  | **MBFC_COMM_EVENT_LOG**        | Get comm event log               | _Serial only_ |
| **X** | 0x0F  | **MBFC_WRITE_MULTIPLE_COILS**  | Write multiple coils             |               |
| **X** | 0x10  | **MBFC_WRITE_MULTIPLE_REGS**   | Write multiple holding registers |               |
|       | 0x11  | **MBFC_REPORT_SLAVE_ID**       | Report slave identification      | _Serial only_ |
|       | 0x14  | **MBFC_READ_FILE_RECORD**      | Read file record                 |               |
|       | 0x15  | **MBFC_WRITE_FILE_RECORD**     | Write file record                |               |
| **X** | 0x16  | **MBFC_MASK_WRITE_REG**        | Mask Write Register              |               |
| **X** | 0x17  | **MBFC_READ_WRITE_REGS**       | Read/Write multiple registers    |               |
|       | 0x18  | **MBFC_READ_FIFO_QUEUE**       | Read FIFO queue                  |               |

## Status Codes

Enum `enum mbstatus_e` can be found in `mbdef.h`.

| Value | Enum                     | Description                                                                         |
| ----- | ------------------------ | ----------------------------------------------------------------------------------- |
| 0x0   | **MB_OK**                | No error                                                                            |
| 0x1   | **MB_ILLEGAL_FN**        | Function code received in the query is not recognized or allowed                    |
| 0x2   | **MB_ILLEGAL_DATA_ADDR** | Data address of some or all the required entities are not allowed or do not exist   |
| 0x3   | **MB_ILLEGAL_DATA_VAL**  | Illegal data value. Value is not accepted                                           |
| 0x4   | **MB_DEV_FAIL**          | Unrecoverable error occurred while slave was attempting to perform requested action |
| 0x5   | **MB_ACK**               |                                                                                     |
| 0x6   | **MB_BUSY**              | Slave is engaged in processing a long-duration command; client should retry later   |
| 0x7   | **MB_NEG_ACK**           |                                                                                     |
| 0x8   | **MB_MEM_PAR_ERR**       | Slave detected a parity error in memory; master can retry the request               |

## Data Types

### Register Types (MRTYPE)

| Type                    | Description       | Size (registers) |
| ----------------------- | ----------------- | ---------------- |
| `MRTYPE_U8/U16/U32/U64` | Unsigned integers | 1/1/2/4          |
| `MRTYPE_I8/I16/I32/I64` | Signed integers   | 1/1/2/4          |
| `MRTYPE_F32/F64`        | Floating point    | 2/4              |
| `MRTYPE_BLOCK`          | Array with offset | Variable         |

For `MRTYPE_BLOCK` with (`MRTYPE_U8` and `MRTYPE_I8`) one register per entry will be used. E.g. `uint8_t reg[4]` will take 4 16-bit registers. The value will simply be cast to a 16-bit value.

> [!Note]
> Multi-register types (U32, U64, F32, F64) use big-endian word order as per Modbus specification, unless `mbinst_s::swap_words` is set (applies only to input registers).

## Access Methods

### Coil Access (MCACC)

| Method         | Description                                           |
| -------------- | ----------------------------------------------------- |
| `MCACC_R_VAL`  | Read constant value from coil descriptor              |
| `MCACC_R_PTR`  | Read value from pointer                               |
| `MCACC_W_PTR`  | Write value to pointer                                |
| `MCACC_RW_PTR` | Read/write via pointer _(MCACC_R_PTR \| MCACC_W_PTR)_ |
| `MCACC_R_FN`   | Read via callback function                            |
| `MCACC_W_FN`   | Write via callback function                           |
| `MCACC_RW_FN`  | Read/write via callbacks _(MCACC_R_FN \| MCACC_W_FN)_ |

### Register Access (MRACC)

| Method         | Description                                           |
| -------------- | ----------------------------------------------------- |
| `MRACC_R_VAL`  | Read constant value from register descriptor          |
| `MRACC_R_PTR`  | Read value from pointer                               |
| `MRACC_W_PTR`  | Write value to pointer                                |
| `MRACC_RW_PTR` | Read/write via pointer _(MRACC_R_PTR \| MRACC_W_PTR)_ |
| `MRACC_R_FN`   | Read via callback function                            |
| `MRACC_W_FN`   | Write via callback function                           |
| `MRACC_RW_FN`  | Read/write via callbacks _(MRACC_R_FN \| MRACC_W_FN)_ |
