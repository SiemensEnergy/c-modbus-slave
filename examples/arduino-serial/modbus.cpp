#include "modbus.hpp"
#include <Arduino.h>
#include <stdint.h>

extern "C" {
#include <mbcoil.h>
#include <mbreg.h>
}

static uint16_t s_some_val = 0x1234u;

static int get_led_state(void)
{
	return digitalRead(LED_BUILTIN) ? 1u : 0u;
}

static enum mbstatus_e set_led_state(int state)
{
	digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
	return MB_OK;
}

static uint32_t read_uptime(void)
{
	return (uint32_t)millis();
}

/**
 * Discrete Inputs (Read-only bits)
 */
static const struct mbcoil_desc_s s_discrete_inputs[] = {
	{
		.address=0x0000,
		.access=MCACC_R_FN,
		.read={.fn=get_led_state}
	}
};

/**
 * Coils (Read/write bits)
 */
static const struct mbcoil_desc_s s_coils[] = {
	{
		.address=0x0000,
		.access=MCACC_RW_FN,
		.read={.fn=get_led_state},
		.write={.fn=set_led_state}
	}
};

/**
 * Input Registers (Read-only 16-bit values)
 */
static const struct mbreg_desc_s s_input_regs[] = {
	{ /* Uptime ms */
		.address=0x0000,
		.type=MRTYPE_U32,
		.access=MRACC_R_FN,
		.read={.fu32=read_uptime}
	},
	{
		.address=0x0002,
		.type=MRTYPE_U16,
		.access=MRACC_R_PTR,
		.read={.pu16=&s_some_val}
	}
};

/**
 * Holding Registers (Read/write 16-bit values)
 */
static const struct mbreg_desc_s s_holding_regs[] = {
	{
		.address=0x0000,
		.type=MRTYPE_U16,
		.access=MRACC_RW_PTR,
		.read={.pu16=&s_some_val},
		.write={.pu16=&s_some_val}
	}
};

/**
 * Modbus Instance
 */
static struct mbinst_s s_mbinst = {
	.disc_inputs = s_discrete_inputs,
	.n_disc_inputs = sizeof s_discrete_inputs / sizeof s_discrete_inputs[0],

	.coils = s_coils,
	.n_coils = sizeof s_coils / sizeof s_coils[0],

	.input_regs = s_input_regs,
	.n_input_regs = sizeof s_input_regs / sizeof s_input_regs[0],

	.hold_regs = s_holding_regs,
	.n_hold_regs = sizeof s_holding_regs / sizeof s_holding_regs[0],

	.handle_fn_cb = NULL,
	.commit_coils_write_cb = NULL,
	.commit_regs_write_cb = NULL,

	.serial = {
		.slave_addr = MODBUS_SLAVE_ADDRESS,
	}
};

extern void modbus_init(void)
{
	/* If descriptor maps are local to another translation unit, you can do something like this */
	/*s_mbinst.input_regs = mbreg_get_input();
	s_mbinst.n_input_regs = mbreg_get_input_count();*/

	mbinst_init(&s_mbinst); /* Initialize internal instance state */
}

extern struct mbinst_s *modbus_get(void)
{
	return &s_mbinst;
}
