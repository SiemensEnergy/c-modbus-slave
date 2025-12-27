# Glossary

## Abbreviations

| Abbr | Name                                        | Description                                                      |
| ---- | ------------------------------------------- | ---------------------------------------------------------------- |
| ADU  | Application Data Unit                       | Complete Modbus frame including slave address and error checking |
| CRC  | Cyclic Redundancy Check                     | Error detection code used in Modbus RTU                          |
| DESC | Descriptor                                  | Configuration structure for coils or registers                   |
| LRC  | Longitudinal Redundancy Check               | Error detection code used in Modbus ASCII                        |
| PDU  | Protocol Data Unit                          | Modbus message without addressing or error checking              |
| RTU  | Remote Terminal Unit                        | Serial implementation of Modbus protocol                         |
| MBAP | Modbus Application Protocol (TCP/IP) Header | Header used in Modbus TCP/IP communications                      |

## Modbus concepts

| Name              | Description                                         |
| ----------------- | --------------------------------------------------- |
| Coil              | Single-bit read/write value (outputs)               |
| Discrete inputs   | Single-bit read-only value (inputs)                 |
| Holding registers | 16-bit read/write value                             |
| Input register    | 16-bit read-only value                              |
| File record       | Organized register data structure with file numbers |
