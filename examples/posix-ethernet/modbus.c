#include "modbus.h"

/*#include <mbcoil.h>*/
#include <mbreg.h>
#include <stdint.h>

static uint16_t h1=0;

static struct mbreg_desc_s s_holding_regs[] = {
	{
		.address=0u,
		.type=MRTYPE_U16,
		.access=MRACC_RW_PTR,
		.read={.pu16=&h1},
		.write={.pu16=&h1}
	}
};

static struct mbinst_s s_mbinst = {
	.hold_regs = s_holding_regs,
	.n_hold_regs = sizeof s_holding_regs / sizeof s_holding_regs[0]
};

extern void modbus_init(void)
{
	mbinst_init(&s_mbinst);
}

extern struct mbinst_s *modbus_get(void)
{
	return &s_mbinst;
}
