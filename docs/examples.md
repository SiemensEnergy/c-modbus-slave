# Examples

Comprehensive usage examples for different scenarios and configurations.

## Basic Examples

### Simple Coil Control

```c
#include "mbinst.h"

/* Define coil data */
static uint8_t s_relay1 = 0u;
static uint8_t s_relay2n3 = 1u; /* Byte containing two coils */

/* Create coil map (must be in ascending address order) */
static const struct mbcoil_desc_s s_coils[] = {
    {
        .address=0x00,
        .access=MCACC_RW_PTR,
        .read={.ptr=&s_relay1},
        .write={.ptr=&s_relay1}
    },
    {
        .address=0x01,
        .access=MCACC_RW_PTR,
        .read={.ptr=&s_relay2n3, .ix=0},
        .write={.ptr=&s_relay2n3, .ix=0}
    },
    {
        .address=0x02,
        .access=MCACC_RW_PTR,
        .read={.ptr=&s_relay2n3, .ix=1},
        .write={.ptr=&s_relay2n3, .ix=1}
    }
};

/* Register coils to instance */
static struct mbinst_s s_inst = {
    .coils=s_coils,
    .n_coils=sizeof s_coils / sizeof s_coils[0]
};
```

### Simple Register Access

```c
#include "mbinst.h"

/* Define register data */
static uint16_t s_temperature = 250;
static uint16_t s_pressure = 1013;

/* Create register map (must be in ascending address order) */
static const struct mbreg_desc_s s_holding_regs[] = {
    {
        .address=0x00,
        .type=MRTYPE_U16,
        .access=MRACC_RW_PTR,
        .read={.pu16=&s_temperature},
        .write={.pu16=&s_temperature}
    },
    {
        .address=0x01,
        .type=MRTYPE_U16,
        .access=MRACC_R_PTR, /* Read-only */
        .read={.pu16=&s_pressure}
    }
};

/* Create instance */
static struct mbinst_s s_inst = {
    .hold_regs=s_holding_regs,
    .n_hold_regs=sizeof s_holding_regs / sizeof s_holding_regs[0]
};
```

## Advanced Data Types

### Mixed Register Types

```c
static uint32_t s_counter = 0;
static float s_sensor_value = 3.14f;
static int16_t s_offset = -100;

static const struct mbreg_desc_s s_mixed_regs[] = {
    /* 32-bit counter (takes 2 registers: 0x10-0x11) */
    {
        .address=0x10,
        .type=MRTYPE_U32,
        .access=MRACC_RW_PTR,
        .read={.pu32=&s_counter},
        .write={.pu32=&s_counter}
    },
    /* Float (single) value (takes 2 registers: 0x20-0x21) */
    {
        .address=0x20,
        .type=MRTYPE_F32,
        .access=MRACC_R_PTR,
        .read={.pf32=&s_sensor_value}
    },
    /* Signed 16-bit offset */
    {
        .address=0x30,
        .type=MRTYPE_I16,
        .access=MRACC_RW_PTR,
        .read={.pi16=&s_offset},
        .write={.pi16=&s_offset}
    }
};
```

### Array Access (Blocks)

```c
static uint16_t s_u16_data_buf[100];
static uint32_t s_u32_data_buf[50];

static const struct mbreg_desc_s s_block_regs[] = {
    {
        .address=0x1000,
        .type=MRTYPE_U16 | MRTYPE_BLOCK,
        .access=MRACC_RW_PTR,
        .read={.pu16=s_u16_data_buf},
        .write={.pu16=s_u16_data_buf},
        .n_block_entries=sizeof s_u16_data_buf / sizeof s_u16_data_buf[0]
    },
    {
        .address=0x1064,
        .type=MRTYPE_U32 | MRTYPE_BLOCK,
        .access=MRACC_RW_PTR,
        .read={.pu16=s_u32_data_buf},
        .write={.pu16=s_u32_data_buf},
        .n_block_entries=sizeof s_u32_data_buf / sizeof s_u32_data_buf[0]
    }
};
/* Reading address 0x1005 will return s_u16_data_buf[5] */
/* Reading address 0x1066 will return s_u32_data_buf[1] */
/* Reading address 0x1067 will return starting from upper byte of s_u32_data_buf[1] */
```

## Callback Functions

### Coil Callbacks

```c
static int s_pump_running = 0;
static int s_safety_enabled = 1;

static int get_pump_status(void)
{
    return !!s_pump_running;
}

static enum mbstatus_e set_pump_status(int value)
{
    if (!s_safety_enabled && value) {
        /* Safety interlock - can't enable pump */
        return MB_ILLEGAL_DATA_VAL; /* Reject write (Prefer using a lock!) */
    }
    s_pump_running = value;
    return MB_OK; /* Success */
}

static const struct mbcoil_desc_s s_callback_coils[] = {
    {
        .address=0x100,
        .access=MCACC_R_FN,
        .read={.fn=get_pump_status}
    },
    {
        .address=0x101,
        .access=MCACC_W_FN,
        .write={.fn=set_pump_status}
    }
};
```

### Register Callbacks

```c
/* Global variables referenced by callbacks */
static int s_system_running = 1;
static uint8_t s_system_mode = 0;

/* Read callback - called when register is read */
static uint16_t get_system_status(void)
{
    return s_system_running ? 0x01 : 0x00;
}

/* Write callback - called when register is written */
static enum mbstatus_e set_system_mode(uint16_t mode)
{
    if (mode > 3) return MB_ILLEGAL_DATA_VAL; /* Invalid mode - reject write */
    s_system_mode = mode;
    return MB_OK; /* Success - accept write */
}

static const struct mbreg_desc_s s_callback_regs[] = {
    {
        .address=0x100,
        .type=MRTYPE_U16,
        .access=MRACC_R_FN,
        .read={.fu16=get_system_status}
    },
    {
        .address=0x101,
        .type=MRTYPE_U16,
        .access=MRACC_W_FN,
        .write={.fu16=set_system_mode}
    }
};
```

## Access Control

### Protected Coils

```c
static uint8_t s_alarm_output = 0u;
static int s_alarm_locked = 1u;

/* Check if coil can be written to */
static int is_alarm_locked(void)
{
    return s_alarm_locked;  /* 1 = locked, 0 = unlocked */
}

/* Called after successful write operation */
static void alarm_written(void)
{
    update_alarm_hardware();
}

static const struct mbcoil_desc_s s_protected_coils[] = {
    {
        .address=0x200,
        .access=MCACC_RW_PTR,
        .read={.ptr=&s_alarm_output},
        .write={.ptr=&s_alarm_output},
        .wlock_cb=is_alarm_locked,
        .post_write_cb=alarm_written
    }
};
```

### Protected Registers

```c
static uint16_t s_config_value = 0x1234;
static int s_config_locked = 1;

/* Check if register can be written to */
static int is_config_locked(void)
{
    return s_config_locked;  /* 1 = locked, 0 = unlocked */
}

/* Called after successful write operation */
static void config_written(void)
{
    save_config_to_flash();
}

static const struct mbreg_desc_s s_protected_regs[] = {
    {
        .address=0x200,
        .type=MRTYPE_U16,
        .access=MRACC_RW_PTR,
        .read={.pu16=&s_config_value},
        .write={.pu16=&s_config_value},
        .wlock_cb=is_config_locked,
        .post_write_cb=config_written
    }
};
```

## Custom Function Handler

```c
static enum mbstatus_e custom_handler(
    const struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    switch (req[0]) {
    case 0x11: /* Report Slave Identification */
        res->p[1] = 3; /* Byte count */
        res->p[2] = 0xFF; /* Slave ID */
        res->p[3] = 0xFF; /* Run status */
        res->p[4] = 0x01; /* Additional data */
        res->size = 5;
        return MB_OK;

    default:
        return MB_ILLEGAL_FN; /* Unknown function */
    }
}

static struct mbinst_s inst = {
    .coils = s_coils,
    .n_coils = sizeof s_coils / sizeof s_coils[0],
    .handle_fn_cb = custom_handler,
};
```
