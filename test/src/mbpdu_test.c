#include "test_lib.h"
#include <endian.h>
#include <mbinst.h>
#include <mbpdu.h>

TEST(mbpdu_read_holding_reg_works)
{
	uint16_t reg_0 = 0x7F;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x01, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_HOLDING_REGS, res[0]);
	ASSERT_EQ(4u, res_size);
	ASSERT_EQ(2u, res[1]); /* Byte count */
	ASSERT_EQ(0x00, res[2]); /* Data H */
	ASSERT_EQ(0x7F, res[3]); /* Data L */

	/* Modify data */
	reg_0 = 0x7654;

	res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_HOLDING_REGS, res[0]);
	ASSERT_EQ(4u, res_size);
	ASSERT_EQ(2u, res[1]); /* Byte count */
	ASSERT_EQ(0x76, res[2]); /* Data H */
	ASSERT_EQ(0x54, res[3]); /* Data L */
}

TEST(mbpdu_read_input_reg_works)
{
	uint16_t reg_0 = 0x7F;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_0},
		}
	};
	struct mbinst_s inst = {
		.input_regs=regs,
		.n_input_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_INPUT_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x01, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_INPUT_REGS, res[0]);
	ASSERT_EQ(4u, res_size);
	ASSERT_EQ(2u, res[1]); /* Byte count */
	ASSERT_EQ(0x00, res[2]); /* Data H */
	ASSERT_EQ(0x7F, res[3]); /* Data L */

	/* Modify data */
	reg_0 = 0x7654;

	res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_INPUT_REGS, res[0]);
	ASSERT_EQ(4u, res_size);
	ASSERT_EQ(2u, res[1]); /* Byte count */
	ASSERT_EQ(0x76, res[2]); /* Data H */
	ASSERT_EQ(0x54, res[3]); /* Data L */
}

TEST(mbpdu_illegal_fn_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		0x00, /* Not an actual function */
		0x00, 0x00, /* Start addr */
		0x00, 0x01, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_FN, res[1]);
}

TEST(mbpdu_write_single_reg_works)
{
	uint16_t reg_0 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_SINGLE_REG,
		0x00, 0x10, /* Start addr */
		0x12, 0x34, /* Data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));

	/* Echo of request w/o fc */
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(0x00, res[1]); /* Start addr H */
	ASSERT_EQ(0x10, res[2]); /* Start addr L */
	ASSERT_EQ(0x12, res[3]); /* Data H*/
	ASSERT_EQ(0x34, res[4]); /* Data L*/

	ASSERT_EQ(0x1234u, reg_0);
}

TEST(mbpdu_write_multiple_regs_works)
{
	uint16_t reg_0 = 0u;
	uint16_t reg_1 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		},
		{
			.address=0x11u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_1},
			.write={.pu16=&reg_1},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x10, /* Start addr */
		0x00, 0x02, /* n regs to write */
		0x04, /* Data byte count */
		0x12, 0x34, /* Data reg 0x10 */
		0x56, 0x78, /* Data reg 0x11 */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));

	/* Echo of request w/o fc */
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(0x00, res[1]); /* Start addr H */
	ASSERT_EQ(0x10, res[2]); /* Start addr L */
	ASSERT_EQ(0x00, res[3]); /* n regs to write H */
	ASSERT_EQ(0x02, res[4]); /* n regs to write L */

	ASSERT_EQ(0x1234u, reg_0);
	ASSERT_EQ(0x5678u, reg_1);
}

TEST(mbpdu_read_nonexistent_holding_reg_fails)
{
	uint16_t reg_0 = 0x7F;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x01, /* Start addr (non-existent) */
		0x00, 0x01, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_read_nonexistent_input_reg_fails)
{
	uint16_t reg_0 = 0x7F;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		}
	};
	struct mbinst_s inst = {
		.input_regs=regs,
		.n_input_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_INPUT_REGS,
		0x00, 0x01, /* Start addr (non-existent) */
		0x00, 0x01, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_read_multiple_holding_regs_works)
{
	uint16_t reg_array[3] = {0x1111, 0x2222, 0x3333};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[0]},
		},
		{
			.address=0x01u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[1]},
		},
		{
			.address=0x02u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[2]},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x03, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(8u, res_size); /* fc + 1 byte count + 6 data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_HOLDING_REGS, res[0]);
	ASSERT_EQ(6u, res[1]); /* Byte count */
	ASSERT_EQ(0x11, res[2]); /* Data 1 H */
	ASSERT_EQ(0x11, res[3]); /* Data 1 L */
	ASSERT_EQ(0x22, res[4]); /* Data 2 H */
	ASSERT_EQ(0x22, res[5]); /* Data 2 L */
	ASSERT_EQ(0x33, res[6]); /* Data 3 H */
	ASSERT_EQ(0x33, res[7]); /* Data 3 L */
}

TEST(mbpdu_read_multiple_input_regs_works)
{
	uint16_t reg_array[3] = {0x1111, 0x2222, 0x3333};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[0]},
		},
		{
			.address=0x01u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[1]},
		},
		{
			.address=0x02u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[2]},
		}
	};
	struct mbinst_s inst = {
		.input_regs=regs,
		.n_input_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_INPUT_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x03, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(8u, res_size); /* fc + 1 byte count + 6 data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_INPUT_REGS, res[0]);
	ASSERT_EQ(6u, res[1]); /* Byte count */
	ASSERT_EQ(0x11, res[2]); /* Data 1 H */
	ASSERT_EQ(0x11, res[3]); /* Data 1 L */
	ASSERT_EQ(0x22, res[4]); /* Data 2 H */
	ASSERT_EQ(0x22, res[5]); /* Data 2 L */
	ASSERT_EQ(0x33, res[6]); /* Data 3 H */
	ASSERT_EQ(0x33, res[7]); /* Data 3 L */
}

TEST(mbpdu_write_to_readonly_reg_fails)
{
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL, /* Read-only */
			.read={.u16=0x1234},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_SINGLE_REG,
		0x00, 0x10, /* Start addr */
		0x56, 0x78, /* Data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_write_nonexistent_reg_fails)
{
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL, /* Read-only */
			.read={.u16=0x1234},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_SINGLE_REG,
		0x00, 0xFF, /* Non-existent addr */
		0x12, 0x34, /* Data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_read_with_invalid_quantity_fails)
{
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL, /* Read-only */
			.read={.u16=0x1234},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x00, /* Invalid quantity (0) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_write_multiple_regs_invalid_byte_count_fails)
{
	uint16_t reg_0 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x10, /* Start addr */
		0x00, 0x01, /* n regs to write */
		0x03, /* Invalid byte count (should be 2) */
		0x12, 0x34, /* Data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);

	ASSERT_EQ(0u, reg_0); /* Not changed */
}

TEST(mbpdu_null_inst_safe)
{
	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x00,
		0x00, 0x01,
	};
	/* Should not crash with NULL inst */
	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(NULL, pdu_data, sizeof pdu_data, res);
	/* No assertions - just testing for crash safety */
}

TEST(mbpdu_empty_pdu_safe)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Should not crash with zero length */
	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, NULL, 0, res);
	/* No assertions - just testing for crash safety */
}

TEST(mbpdu_read_partial_register_range_works)
{
	uint16_t reg_array[5] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x05u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[0]},
		},
		{
			.address=0x06u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[1]},
		},
		{
			.address=0x07u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[2]},
		},
		{
			.address=0x08u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[3]},
		},
		{
			.address=0x09u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_array[4]},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Read middle 3 registers */
	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x06, /* Start addr */
		0x00, 0x03, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(8u, res_size); /* fc + 1 byte count + 6 data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_HOLDING_REGS, res[0]);
	ASSERT_EQ(6u, res[1]); /* Byte count */
	ASSERT_EQ(0x22, res[2]); /* Data 1 H */
	ASSERT_EQ(0x22, res[3]); /* Data 1 L */
	ASSERT_EQ(0x33, res[4]); /* Data 2 H */
	ASSERT_EQ(0x33, res[5]); /* Data 2 L */
	ASSERT_EQ(0x44, res[6]); /* Data 3 H */
	ASSERT_EQ(0x44, res[7]); /* Data 3 L */
}

TEST(mbpdu_write_across_register_gap_fails)
{
	uint16_t reg_0 = 0u;
	uint16_t reg_2 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		},
		/* Gap at 0x11 */
		{
			.address=0x12u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_2},
			.write={.pu16=&reg_2},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x10, /* Start addr */
		0x00, 0x03, /* n regs to write (spans gap) */
		0x06, /* Data byte count */
		0x12, 0x34, /* Data 1 */
		0x56, 0x78, /* Data 2 (gap) */
		0x9A, 0xBC, /* Data 3 */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);

	/* Should not modify data */
	ASSERT_EQ(0u, reg_0);
	ASSERT_EQ(0u, reg_2);
}

TEST(mbpdu_read_max_quantity_holding_regs)
{
	uint16_t reg_0 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x7D, /* Max quantity: 125 */
	};

	/* So long as the start addr exists, the rest will just be padded with arbitrary data */
	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
}

TEST(mbpdu_read_max_quantity_input_regs)
{
	uint16_t reg_0 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_0},
		},
	};
	struct mbinst_s inst = {
		.input_regs=regs,
		.n_input_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_INPUT_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x7D, /* Max quantity: 125 */
	};

	/* So long as the start addr exists, the rest will just be padded with arbitrary data */
	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
}

TEST(mbpdu_read_excess_quantity_holding_regs_fails)
{
	uint16_t reg_0 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x7E, /* Quantity: 126 (exceeds max) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_excess_quantity_input_regs_fails)
{
	uint16_t reg_0 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&reg_0},
		},
	};
	struct mbinst_s inst = {
		.input_regs=regs,
		.n_input_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_INPUT_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x7E, /* Quantity: 126 (exceeds max) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_write_zero_quantity_fails)
{
	uint16_t reg_0 = 0u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_0},
			.write={.pu16=&reg_0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x00,
		0x00, 0x00, /* Zero quantity - invalid */
		0x00, /* Zero byte count */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_u32_reg_by_pointer)
{
	uint32_t reg_val = 0x12345678;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U32,
			.access=MRACC_R_PTR,
			.read={.pu32=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_request[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x10, /* Start address */
		0x00, 0x02, /* Quantity: 2 registers (U32 = 2 x 16-bit words) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_request, sizeof pdu_request, res);

	ASSERT_EQ(6, res_size); /* fc + 1 byte count + 4 data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(4, res[1]); /* Byte count */
	/* Verify big-endian encoding of 0x12345678 */
	ASSERT_EQ(0x12, res[2]);
	ASSERT_EQ(0x34, res[3]);
	ASSERT_EQ(0x56, res[4]);
	ASSERT_EQ(0x78, res[5]);
}

TEST(mbpdu_read_float_reg_by_pointer)
{
	float reg_val = 3.14159f;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0020,
			.type=MRTYPE_F32,
			.access=MRACC_R_PTR,
			.read={.pf32=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_request[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x20, /* Start address */
		0x00, 0x02, /* Quantity: 2 registers (F32 = 2 x 16-bit words) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_request, sizeof pdu_request, res);

	ASSERT_EQ(6, res_size); /* fc + 1 byte count + 4 data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(4, res[1]); /* Byte count */
	/* Float encoding should produce valid IEEE 754 big-endian bytes */
	ASSERT_EQ(reg_val, betof32(res+2));
}

static uint16_t test_read_callback(void)
{
	return 0xABCD;
}
TEST(mbpdu_read_reg_by_callback)
{
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0030,
			.type=MRTYPE_U16,
			.access=MRACC_R_FN,
			.read={.fu16=test_read_callback},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_request[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x30, /* Start address */
		0x00, 0x01, /* Quantity: 1 register */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_request, sizeof pdu_request, res);

	ASSERT_EQ(4, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(2, res[1]); /* Byte count */
	ASSERT_EQ(0xAB, res[2]);
	ASSERT_EQ(0xCD, res[3]);
}

static uint16_t s_test_write_callback_u16 = 0x1234;
static enum mbstatus_e test_write_callback(uint16_t value)
{
	s_test_write_callback_u16 = value;
	return MB_OK;
}
TEST(mbpdu_write_reg_by_callback)
{
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0040,
			.type=MRTYPE_U16,
			.access=MRACC_W_FN,
			.write={.fu16=test_write_callback},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_request[] = {
		MBFC_WRITE_SINGLE_REG,
		0x00, 0x40, /* Address */
		0x55, 0xAA, /* Value to write */
	};

	s_test_write_callback_u16 = 0u;

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_request, sizeof pdu_request, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	/* Verify callback was called with correct value */
	ASSERT_EQ(0x55AA, s_test_write_callback_u16);
}

static int s_test_is_locked = 0;
static int test_lock_callback(void)
{
	return s_test_is_locked;
}
TEST(mbpdu_read_locked_reg_access_denied)
{
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0050,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0x1234u},
			.rlock_cb=test_lock_callback,
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_request[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x50, /* Address */
		0x00, 0x01, /* Quantity */
	};

	s_test_is_locked = 1; /* Lock the register */

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_request, sizeof pdu_request, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_write_locked_reg_access_denied)
{
	uint16_t reg_val = 0x1111u;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0060,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
			.wlock_cb=test_lock_callback,
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_request[] = {
		MBFC_WRITE_SINGLE_REG,
		0x00, 0x60, /* Address */
		0x22, 0x22, /* Value */
	};

	s_test_is_locked = 1; /* Lock the register */

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_request, sizeof pdu_request, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
	ASSERT_EQ(0x1111u, reg_val); /* Value should remain unchanged */
}

static int s_post_write_count = 0;
static void test_post_write_callback(void)
{
	++s_post_write_count;
}
TEST(mbpdu_post_write_callback_executed)
{
	uint16_t reg_val = 0;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0070,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
			.post_write_cb=test_post_write_callback,
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_request[] = {
		MBFC_WRITE_SINGLE_REG,
		0x00, 0x70, /* Address */
		0x12, 0x34, /* Value */
	};

	s_post_write_count = 0; /* Reset counter */

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_request, sizeof pdu_request, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x1234u, reg_val);
	ASSERT_EQ(1, s_post_write_count); /* Callback should be executed once */
}

static int s_commit_count = 0;
static void test_commit_callback(const struct mbinst_s *inst)
{
	(void)inst;
	++s_commit_count;
}
TEST(mbpdu_inst_commit_callback_executed)
{
	uint16_t reg_val_1 = 0;
	uint16_t reg_val_2 = 0;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0080,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val_1},
			.write={.pu16=&reg_val_1},
		},
		{
			.address=0x0081,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val_2},
			.write={.pu16=&reg_val_2},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0],
		.commit_regs_write_cb=test_commit_callback,
	};
	mbinst_init(&inst);

	uint8_t pdu_request[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x80, /* Start address */
		0x00, 0x02, /* n regs to write */
		0x04, /* Data byte count */
		0x12, 0x34, /* Data reg 0x80 */
		0x56, 0x78, /* Data reg 0x81 */
	};

	s_commit_count = 0; /* Reset counter */

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_request, sizeof pdu_request, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x1234u, reg_val_1);
	ASSERT_EQ(0x5678u, reg_val_2);
	ASSERT_EQ(1, s_commit_count); /* Should only have been called once */
}

TEST(mbpdu_block_u16_read_works)
{
	uint16_t block_u16_data[10] = {0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007, 0x1008, 0x1009};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000u,
			.type=MRTYPE_U16 | MRTYPE_BLOCK,
			.access=MRACC_R_PTR,
			.read={.pu16=block_u16_data},
			.n_block_entries=10,
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Read first 3 registers from block */
	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x10, 0x00, /* Start addr */
		0x00, 0x03, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(8u, res_size); /* 1 byte count + 6 data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(6u, res[1]); /* Byte count */
	ASSERT_EQ(0x10, res[2]); /* First value H */
	ASSERT_EQ(0x00, res[3]); /* First value L */
	ASSERT_EQ(0x10, res[4]); /* Second value H */
	ASSERT_EQ(0x01, res[5]); /* Second value L */
	ASSERT_EQ(0x10, res[6]); /* Third value H */
	ASSERT_EQ(0x02, res[7]); /* Third value L */
}

TEST(mbpdu_block_u16_read_middle_works)
{
	uint16_t block_u16_data[10] = {0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007, 0x1008, 0x1009};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000u,
			.type=MRTYPE_U16 | MRTYPE_BLOCK,
			.access=MRACC_R_PTR,
			.read={.pu16=block_u16_data},
			.n_block_entries=10,
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Read from middle of block (address 0x1005 = index 5) */
	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x10, 0x05, /* Start addr */
		0x00, 0x02, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(6u, res_size); /* fc + 1 byte count + 4 data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(4u, res[1]); /* Byte count */
	ASSERT_EQ(0x10, res[2]); /* Fifth value H */
	ASSERT_EQ(0x05, res[3]); /* Fifth value L */
	ASSERT_EQ(0x10, res[4]); /* Sixth value H */
	ASSERT_EQ(0x06, res[5]); /* Sixth value L */
}

TEST(mbpdu_block_u32_read_works)
{
	uint32_t block_u32_data[5] = {0x20000000, 0x20000001, 0x20000002, 0x20000003, 0x20000004};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x2000u,
			.type=MRTYPE_U32 | MRTYPE_BLOCK,
			.access=MRACC_R_PTR,
			.read={.pu32=block_u32_data},
			.n_block_entries=5,
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Read second U32 (at register address 0x2002) */
	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x20, 0x02, /* Start addr */
		0x00, 0x02, /* n read regs (1 U32 = 2 regs) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(6u, res_size); /* fc + 1 byte count + 4 data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(4u, res[1]); /* Byte count */
	ASSERT_EQ(0x20, res[2]); /* Data byte 0 */
	ASSERT_EQ(0x00, res[3]); /* Data byte 1 */
	ASSERT_EQ(0x00, res[4]); /* Data byte 2 */
	ASSERT_EQ(0x01, res[5]); /* Data byte 3 */
}

TEST(mbpdu_block_read_beyond_end_fails)
{
	uint16_t block_u16_data[10] = {0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007, 0x1008, 0x1009};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000u,
			.type=MRTYPE_U16 | MRTYPE_BLOCK,
			.access=MRACC_R_PTR,
			.read={.pu16=block_u16_data},
			.n_block_entries=10, /* block ends at 0x1009 */
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Try to read past end of block */
	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x10, 0x0A, /* Start addr (beyond end) */
		0x00, 0x01, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_block_write_works)
{
	uint16_t writable_block[5] = {0, 0, 0, 0, 0};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x3000u,
			.type=MRTYPE_U16 | MRTYPE_BLOCK,
			.access=MRACC_RW_PTR,
			.read={.pu16=writable_block},
			.write={.pu16=writable_block},
			.n_block_entries=5,
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Write to middle of block */
	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x30, 0x02, /* Start addr (index 2) */
		0x00, 0x02, /* n regs to write */
		0x04, /* Data byte count */
		0xAA, 0xBB, /* Data 1 */
		0xCC, 0xDD, /* Data 2 */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0, writable_block[0]); /* Unchanged */
	ASSERT_EQ(0, writable_block[1]); /* Unchanged */
	ASSERT_EQ(0xAABB, writable_block[2]);
	ASSERT_EQ(0xCCDD, writable_block[3]);
	ASSERT_EQ(0, writable_block[4]); /* Unchanged */
}

TEST(mbpdu_block_register_out_of_bounds)
{
	uint16_t block_data[5] = {0x1000, 0x1001, 0x1002, 0x1003, 0x1004};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000,
			.type=MRTYPE_U16 | MRTYPE_BLOCK,
			.access=MRACC_R_PTR,
			.read={.pu16=block_data},
			.n_block_entries=5 /* block has 5 entries: 0x1000-0x1004 */
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Try to read past the end of the block */
	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x10, 0x05, /* Start addr 0x1005 (beyond block end) */
		0x00, 0x01, /* Quantity: 1 register */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_block_read_u8_works)
{
	uint8_t block_data[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000,
			.type=MRTYPE_U8 | MRTYPE_BLOCK,
			.access=MRACC_R_PTR,
			.read={.pu8=block_data},
			.n_block_entries=5 /* block has 5 entries: 0x1000-0x1004 */
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x10, 0x00, /* Start addr */
		0x00, 0x05, /* Quantity register */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(12, res_size); /* fc + byte count + 5 regs */
	ASSERT_EQ(10, res[1]); /* byte count (5 regs) */
	ASSERT_EQ(0x12, betou16(res+2));
	ASSERT_EQ(0x34, betou16(res+4));
	ASSERT_EQ(0x56, betou16(res+6));
	ASSERT_EQ(0x78, betou16(res+8));
	ASSERT_EQ(0x9A, betou16(res+10));
}

TEST(mbpdu_block_write_u8_works)
{
	uint8_t block_data[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000,
			.type=MRTYPE_U8 | MRTYPE_BLOCK,
			.access=MRACC_RW_PTR,
			.read={.pu8=block_data},
			.write={.pu8=block_data},
			.n_block_entries=5 /* block has 5 entries: 0x1000-0x1004 */
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x10, 0x00, /* Start addr */
		0x00, 0x03, /* n regs to write */
		0x06, /* Byte count */
		0x00, 0xFE, /* Reg 0x1000 */
		0x00, 0xDC, /* Reg 0x1001 */
		0x00, 0xBA, /* Reg 0x1002 */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x10, res[1]) /* Start addr H */
	ASSERT_EQ(0x00, res[2]) /* Start addr L */
	ASSERT_EQ(0x00, res[3]) /* n regs to write H */
	ASSERT_EQ(0x03, res[4]) /* n regs to write L */

	ASSERT_EQ(0xFE, block_data[0]);
	ASSERT_EQ(0xDC, block_data[1]);
	ASSERT_EQ(0xBA, block_data[2]);
	ASSERT_EQ(0x78, block_data[3]); /* Not changed */
	ASSERT_EQ(0x9A, block_data[4]); /* Not changed */
}

TEST(mbpdu_block_write_u8_upper_byte_ignored)
{
	uint8_t block_data[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000,
			.type=MRTYPE_U8 | MRTYPE_BLOCK,
			.access=MRACC_RW_PTR,
			.read={.pu8=block_data},
			.write={.pu8=block_data},
			.n_block_entries=5 /* block has 5 entries: 0x1000-0x1004 */
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x10, 0x00, /* Start addr */
		0x00, 0x03, /* n regs to write */
		0x06, /* Byte count */
		0x12, /*< Ignored */ 0xFE, /* Reg 0x1000 */
		0x21, /*< Ignored */ 0xDC, /* Reg 0x1001 */
		0x43, /*< Ignored */ 0xBA, /* Reg 0x1002 */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x10, res[1]) /* Start addr H */
	ASSERT_EQ(0x00, res[2]) /* Start addr L */
	ASSERT_EQ(0x00, res[3]) /* n regs to write H */
	ASSERT_EQ(0x03, res[4]) /* n regs to write L */

	ASSERT_EQ(0xFE, block_data[0]);
	ASSERT_EQ(0xDC, block_data[1]);
	ASSERT_EQ(0xBA, block_data[2]);
	ASSERT_EQ(0x78, block_data[3]); /* Not changed */
	ASSERT_EQ(0x9A, block_data[4]); /* Not changed */
}

TEST(mbpdu_block_read_i8_works)
{
	int8_t block_data[5] = {13, 127, -123, -43, 43};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000,
			.type=MRTYPE_I8 | MRTYPE_BLOCK,
			.access=MRACC_R_PTR,
			.read={.pi8=block_data},
			.n_block_entries=5 /* block has 5 entries: 0x1000-0x1004 */
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x10, 0x00, /* Start addr */
		0x00, 0x05, /* Quantity register */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(12, res_size); /* fc + byte count + 5 regs */
	ASSERT_EQ(10, res[1]); /* byte count (5 regs) */
	ASSERT_EQ(13, betoi16(res+2));
	ASSERT_EQ(127, betoi16(res+4));
	ASSERT_EQ(-123, betoi16(res+6));
	ASSERT_EQ(-43, betoi16(res+8));
	ASSERT_EQ(43, betoi16(res+10));
}

TEST(mbpdu_block_write_i8_works)
{
	int8_t block_data[5] = {13, 127, -123, -43, 43};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000,
			.type=MRTYPE_U8 | MRTYPE_BLOCK,
			.access=MRACC_RW_PTR,
			.read={.pi8=block_data},
			.write={.pi8=block_data},
			.n_block_entries=5 /* block has 5 entries: 0x1000-0x1004 */
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x10, 0x00, /* Start addr */
		0x00, 0x03, /* n regs to write */
		0x06, /* Byte count */
		0x00, 0x00, /* Reg 0x1000 (Placeholder) */
		0x00, 0x00, /* Reg 0x1001 (Placeholder) */
		0x00, 0x00, /* Reg 0x1002 (Placeholder) */
	};
	i16tobe(-122, pdu_data+6);
	i16tobe(122, pdu_data+8);
	i16tobe(-42, pdu_data+10);

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x10, res[1]) /* Start addr H */
	ASSERT_EQ(0x00, res[2]) /* Start addr L */
	ASSERT_EQ(0x00, res[3]) /* n regs to write H */
	ASSERT_EQ(0x03, res[4]) /* n regs to write L */

	ASSERT_EQ(-122, (int)block_data[0]);
	ASSERT_EQ(122, (int)block_data[1]);
	ASSERT_EQ(-42, (int)block_data[2]);
	ASSERT_EQ(-43, (int)block_data[3]); /* Not changed */
	ASSERT_EQ(43, (int)block_data[4]); /* Not changed */
}

static int s_custom_handler_called = 0;
static uint8_t s_last_custom_fc = 0;
static enum mbstatus_e custom_function_handler(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	(void)inst;
	(void)req;
	(void)req_len;

	++s_custom_handler_called;
	s_last_custom_fc = req[0];

	if (req[0] == 0x41) { /* Custom read function */
		res->p[1] = 4; /* Byte count */
		res->p[2] = 0xDE;
		res->p[3] = 0xAD;
		res->p[4] = 0xBE;
		res->p[5] = 0xEF;
		res->size = 6;
		return MB_OK;
	}

	return MB_ILLEGAL_FN;
}

TEST(mbpdu_custom_function_handler_works)
{
	struct mbinst_s inst = {
		.handle_fn_cb=custom_function_handler,
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		0x41, /* Custom function code */
		0x12, 0x34, /* Some data */
	};

	s_custom_handler_called = 0;

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(1, s_custom_handler_called);
	ASSERT_EQ(0x41, s_last_custom_fc);

	ASSERT_EQ(6, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x41, res[0]);
	ASSERT_EQ(4, res[1]); /* Byte count */
	ASSERT_EQ(0xDE, res[2]);
	ASSERT_EQ(0xAD, res[3]);
	ASSERT_EQ(0xBE, res[4]);
	ASSERT_EQ(0xEF, res[5]);
}

TEST(mbpdu_custom_handler_fallback_for_standard_functions)
{
	/* inst with no registers, so standard functions should fall through to custom handler */
	struct mbinst_s inst = {
		.handle_fn_cb=custom_function_handler,
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x00,
		0x00, 0x01,
	};

	s_custom_handler_called = 0;

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(1, s_custom_handler_called);
	ASSERT_EQ(MBFC_READ_HOLDING_REGS, s_last_custom_fc);
	ASSERT(res[0] & MB_ERR_FLG); /* Custom handler returns ILL_FN for standard functions */
}

TEST(mbpdu_word_order_swap_u32_works)
{
	static uint32_t swap_test_u32 = 0x12345678;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x4000u,
			.type=MRTYPE_U32,
			.access=MRACC_R_PTR,
			.read={.pu32=&swap_test_u32},
		}
	};

	struct mbinst_s inst = {
		.input_regs=regs,
		.n_input_regs=sizeof regs / sizeof regs[0],
		.swap_words=1, /* Enable word order swapping */
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_INPUT_REGS,
		0x40, 0x00, /* Start addr */
		0x00, 0x02, /* n read regs (U32 = 2 regs) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(6u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(4u, res[1]); /* Byte count */

	/* With word order swap, low word should come first */
	/* Original: 0x12345678 -> words: 0x1234, 0x5678 -> swapped: 0x5678, 0x1234 */
	ASSERT_EQ(0x56, res[2]); /* Low word H */
	ASSERT_EQ(0x78, res[3]); /* Low word L */
	ASSERT_EQ(0x12, res[4]); /* High word H */
	ASSERT_EQ(0x34, res[5]); /* High word L */
}

TEST(mbpdu_word_order_swap_u64_works)
{
	static uint64_t swap_test_u64 = 0x123456789ABCDEF0uLL;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x4000u,
			.type=MRTYPE_U64,
			.access=MRACC_R_PTR,
			.read={.pu64=&swap_test_u64},
		}
	};

	struct mbinst_s inst = {
		.input_regs=regs,
		.n_input_regs=sizeof regs / sizeof regs[0],
		.swap_words=1, /* Enable word order swapping */
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_INPUT_REGS,
		0x40, 0x00, /* Start addr */
		0x00, 0x04, /* n read regs (U64 = 4 regs) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(10u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(8u, res[1]); /* Byte count */

	ASSERT_EQ(0xDE, res[2]);
	ASSERT_EQ(0xF0, res[3]);
	ASSERT_EQ(0x9A, res[4]);
	ASSERT_EQ(0xBC, res[5]);
	ASSERT_EQ(0x56, res[6]);
	ASSERT_EQ(0x78, res[7]);
	ASSERT_EQ(0x12, res[8]);
	ASSERT_EQ(0x34, res[9]);
}

TEST(mbpdu_mixed_register_types_work)
{
	static uint8_t test_u8 = 0x42;
	static uint16_t test_u16 = 0x1234;
	static uint32_t test_u32 = 0x56789ABC;

	const struct mbreg_desc_s regs[] = {
		{
			.address=0x5000u,
			.type=MRTYPE_U8,
			.access=MRACC_R_PTR,
			.read={.pu8=&test_u8},
		},
		{
			.address=0x5001u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_u16},
		},
		{
			.address=0x5002u, /* U32 takes 2 registers: 0x5002, 0x5003 */
			.type=MRTYPE_U32,
			.access=MRACC_R_PTR,
			.read={.pu32=&test_u32},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Read U8 (which takes 1 register but returns 2 bytes) */
	uint8_t pdu_data1[] = {
		MBFC_READ_HOLDING_REGS,
		0x50, 0x00, /* Start addr */
		0x00, 0x01, /* n read regs */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data1, sizeof pdu_data1, res);
	ASSERT_EQ(4u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(2u, res[1]); /* Byte count */
	ASSERT_EQ(0x00, res[2]); /* U8 padded to 16-bit H */
	ASSERT_EQ(0x42, res[3]); /* U8 value L */

	/* Read U16 */
	uint8_t pdu_data2[] = {
		MBFC_READ_HOLDING_REGS,
		0x50, 0x01, /* Start addr */
		0x00, 0x01, /* n read regs */
	};

	res_size = mbpdu_handle_req(&inst, pdu_data2, sizeof pdu_data2, res);
	ASSERT_EQ(4u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(2u, res[1]); /* Byte count */
	ASSERT_EQ(0x12, res[2]); /* U16 H */
	ASSERT_EQ(0x34, res[3]); /* U16 L */

	/* Read U32 */
	uint8_t pdu_data3[] = {
		MBFC_READ_HOLDING_REGS,
		0x50, 0x02, /* Start addr */
		0x00, 0x02, /* n read regs (U32 = 2 regs) */
	};

	res_size = mbpdu_handle_req(&inst, pdu_data3, sizeof pdu_data3, res);
	ASSERT_EQ(6u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(4u, res[1]); /* Byte count */
	ASSERT_EQ(0x56, res[2]); /* U32 byte 0 */
	ASSERT_EQ(0x78, res[3]); /* U32 byte 1 */
	ASSERT_EQ(0x9A, res[4]); /* U32 byte 2 */
	ASSERT_EQ(0xBC, res[5]); /* U32 byte 3 */
}

TEST(mbpdu_read_holding_regs_invalid_quantity_zero)
{
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0000,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0x1234}
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Try to read 0 registers */
	uint8_t pdu_data[] = {
		MBFC_READ_HOLDING_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x00, /* Quantity: 0 */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_write_single_reg_invalid_address)
{
	uint16_t test_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0000,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&test_val},
			.write={.pu16=&test_val}
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Try to write to non-existent address */
	uint8_t pdu_data[] = {
		MBFC_WRITE_SINGLE_REG,
		0x10, 0x00, /* Address 0x1000 (doesn't exist) */
		0x56, 0x78, /* Value */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_write_multiple_regs_mismatched_byte_count)
{
	uint16_t test_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0000,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&test_val},
			.write={.pu16=&test_val}
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Malformed request - claiming 2 registers but only providing 1 register worth of data */
	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x00, /* Start addr */
		0x00, 0x02, /* Quantity: 2 registers */
		0x02,       /* Byte count: 2 (should be 4 for 2 registers) */
		0x12, 0x34  /* Only data for 1 register */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_write_regs_basic_works)
{
	uint16_t read_reg = 0x1234;
	uint16_t write_reg = 0x0000;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&read_reg},
		},
		{
			.address=0x0020,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&write_reg},
			.write={.pu16=&write_reg},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Read start addr */
		0x00, 0x01, /* Read quantity */
		0x00, 0x20, /* Write start addr */
		0x00, 0x01, /* Write quantity */
		0x02, /* Write byte count */
		0x56, 0x78, /* Write data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(4u, res_size); /* fc + 1 byte count + 2 read data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_WRITE_REGS, res[0]);
	ASSERT_EQ(2u, res[1]); /* Read byte count */
	ASSERT_EQ(0x12, res[2]); /* Read data H */
	ASSERT_EQ(0x34, res[3]); /* Read data L */

	/* Verify write operation occurred */
	ASSERT_EQ(0x5678, write_reg);
}

TEST(mbpdu_read_write_regs_multiple_works)
{
	uint16_t read_regs[2] = {0x1111, 0x2222};
	uint16_t write_regs[2] = {0x0000, 0x0000};
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0100,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&read_regs[0]},
		},
		{
			.address=0x0101,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&read_regs[1]},
		},
		{
			.address=0x0200,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&write_regs[0]},
			.write={.pu16=&write_regs[0]},
		},
		{
			.address=0x0201,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&write_regs[1]},
			.write={.pu16=&write_regs[1]},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x01, 0x00, /* Read start addr */
		0x00, 0x02, /* Read quantity */
		0x02, 0x00, /* Write start addr */
		0x00, 0x02, /* Write quantity */
		0x04, /* Write byte count */
		0xAA, 0xBB, /* Write data 1 */
		0xCC, 0xDD, /* Write data 2 */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(6u, res_size); /* 1 byte count + 4 read data bytes */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_WRITE_REGS, res[0]);
	ASSERT_EQ(4u, res[1]); /* Read byte count */
	ASSERT_EQ(0x11, res[2]); /* Read data 1 H */
	ASSERT_EQ(0x11, res[3]); /* Read data 1 L */
	ASSERT_EQ(0x22, res[4]); /* Read data 2 H */
	ASSERT_EQ(0x22, res[5]); /* Read data 2 L */

	/* Verify write operations occurred */
	ASSERT_EQ(0xAABB, write_regs[0]);
	ASSERT_EQ(0xCCDD, write_regs[1]);
}

TEST(mbpdu_read_write_regs_same_register_works)
{
	uint16_t reg_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0050,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x50, /* Read start addr (same as write) */
		0x00, 0x01, /* Read quantity */
		0x00, 0x50, /* Write start addr (same as read) */
		0x00, 0x01, /* Write quantity */
		0x02, /* Write byte count */
		0x56, 0x78, /* Write data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(4u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_WRITE_REGS, res[0]);
	ASSERT_EQ(2u, res[1]); /* Read byte count */
	/* Should return the NEW value since write happens first */
	ASSERT_EQ(0x56, res[2]); /* New data H */
	ASSERT_EQ(0x78, res[3]); /* New data L */

	/* Verify write operation occurred */
	ASSERT_EQ(0x5678, reg_val);
}

TEST(mbpdu_read_write_regs_invalid_request_length_fails)
{
	uint16_t reg_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Request too short - missing some fields */
	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Read start addr */
		0x00, 0x01, /* Read quantity */
		0x00, 0x10, /* Write start addr */
		/* Missing write quantity and data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_write_regs_byte_count_mismatch_fails)
{
	uint16_t reg_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Read start addr */
		0x00, 0x01, /* Read quantity */
		0x00, 0x10, /* Write start addr */
		0x00, 0x01, /* Write quantity */
		0x04, /* Wrong byte count (should be 2) */
		0x56, 0x78, /* Write data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
	/* Original value should be unchanged */
	ASSERT_EQ(0x1234, reg_val);
}

TEST(mbpdu_read_write_regs_nonexistent_read_addr_fails)
{
	uint16_t reg_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0020,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Non-existent read addr */
		0x00, 0x01, /* Read quantity */
		0x00, 0x20, /* Valid write addr */
		0x00, 0x01, /* Write quantity */
		0x02, /* Write byte count */
		0x56, 0x78, /* Write data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
	/* Write should not have occurred */
	ASSERT_EQ(0x1234, reg_val);
}

TEST(mbpdu_read_write_regs_nonexistent_write_addr_fails)
{
	uint16_t reg_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Valid read addr */
		0x00, 0x01, /* Read quantity */
		0x00, 0x20, /* Non-existent write addr */
		0x00, 0x01, /* Write quantity */
		0x02, /* Write byte count */
		0x56, 0x78, /* Write data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
	/* Original value should be unchanged */
	ASSERT_EQ(0x1234, reg_val);
}

TEST(mbpdu_read_write_regs_write_readonly_fails)
{
	uint16_t read_reg = 0x1234;
	uint16_t readonly_reg = 0x5678;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&read_reg},
		},
		{
			.address=0x0020,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR, /* Read-only */
			.read={.pu16=&readonly_reg},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Valid read addr */
		0x00, 0x01, /* Read quantity */
		0x00, 0x20, /* Read-only write addr */
		0x00, 0x01, /* Write quantity */
		0x02, /* Write byte count */
		0xAA, 0xBB, /* Write data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
	/* Values should be unchanged */
	ASSERT_EQ(0x1234, read_reg);
	ASSERT_EQ(0x5678, readonly_reg);
}

static int s_read_write_commit_count = 0;
static void test_read_write_commit_callback(const struct mbinst_s *inst)
{
	(void)inst;
	++s_read_write_commit_count;
}

TEST(mbpdu_read_write_regs_commit_callback_executed)
{
	uint16_t read_reg = 0x1111;
	uint16_t write_reg = 0x0000;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&read_reg},
		},
		{
			.address=0x0020,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&write_reg},
			.write={.pu16=&write_reg},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0],
		.commit_regs_write_cb=test_read_write_commit_callback
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Read start addr */
		0x00, 0x01, /* Read quantity */
		0x00, 0x20, /* Write start addr */
		0x00, 0x01, /* Write quantity */
		0x02, /* Write byte count */
		0x22, 0x33, /* Write data */
	};

	s_read_write_commit_count = 0; /* Reset counter */

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x2233, write_reg);
	ASSERT_EQ(1, s_read_write_commit_count); /* Should be called once */
}

TEST(mbpdu_read_write_regs_zero_read_quantity_fails)
{
	uint16_t reg_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Read start addr */
		0x00, 0x00, /* Read quantity = 0 (invalid) */
		0x00, 0x10, /* Write start addr */
		0x00, 0x01, /* Write quantity */
		0x02, /* Write byte count */
		0x56, 0x78, /* Write data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_write_regs_zero_write_quantity_fails)
{
	uint16_t reg_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0010,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg_val},
			.write={.pu16=&reg_val},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_WRITE_REGS,
		0x00, 0x10, /* Read start addr */
		0x00, 0x01, /* Read quantity */
		0x00, 0x10, /* Write start addr */
		0x00, 0x00, /* Write quantity = 0 (invalid) */
		0x00, /* Write byte count = 0 */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	(void)mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

static uint32_t s_test_write_callback_u32 = 0u;
static enum mbstatus_e test_write_callback_u32(uint32_t value)
{
	s_test_write_callback_u32 = value;
	return MB_OK;
}
TEST(mbpdu_write_partial_reg_fc_cb_fails)
{
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U32,
			.access=MRACC_W_FN,
			.write={.fu32=test_write_callback_u32},
		}
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t res[MBPDU_SIZE_MAX];

	/* Fails lower part */
	uint8_t pdu1_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x10, /* Start addr */
		0x00, 0x01, /* n regs to write */
		0x02, /* Data byte count */
		0x12, 0x34}; /* Data reg 0x10 */
	size_t res_size = mbpdu_handle_req(&inst, pdu1_data, sizeof pdu1_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
	ASSERT_EQ(0, s_test_write_callback_u32);

	/* Fails upper part */
	uint8_t pdu2_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x11, /* Start addr */
		0x00, 0x01, /* n regs to write */
		0x02, /* Data byte count */
		0x56, 0x78}; /* Data reg 0x11 */
	res_size = mbpdu_handle_req(&inst, pdu2_data, sizeof pdu2_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
	ASSERT_EQ(0, s_test_write_callback_u32);

	/* Works entire register */
	uint8_t pdu3_data[] = {
		MBFC_WRITE_MULTIPLE_REGS,
		0x00, 0x10, /* Start addr */
		0x00, 0x02, /* n regs to write */
		0x04, /* Data byte count */
		0x12, 0x34, /* Data reg 0x10 */
		0x56, 0x78}; /* Data reg 0x11 */
	res_size = mbpdu_handle_req(&inst, pdu3_data, sizeof pdu3_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x00, res[1]) /* Start addr H */
	ASSERT_EQ(0x10, res[2]) /* Start addr L */
	ASSERT_EQ(0x00, res[3]) /* n regs to write H */
	ASSERT_EQ(0x02, res[4]) /* n regs to write L */
	ASSERT_EQ(0x12345678u, s_test_write_callback_u32);
}

TEST(mbpdu_mask_write_res_works)
{
	uint16_t reg0 = 0b00010010;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg0},
			.write={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x00, 0b11110010, /* AND mask */
		0x00, 0b00100101, /* OR mask */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	ASSERT_EQ(0x00, res[1]); /* Addr H */
	ASSERT_EQ(0x00, res[2]); /* Addr L */
	ASSERT_EQ(0x00, res[3]); /* AND mask H */
	ASSERT_EQ(0b11110010, res[4]); /* AND mask L */
	ASSERT_EQ(0x00, res[5]); /* OR mask H */
	ASSERT_EQ(0b00100101, res[6]); /* OR mask L */

	/* Check result */
	ASSERT_EQ(0b00010111, reg0);
}

TEST(mbpdu_mask_write_invalid_request_length_fails)
{
	uint16_t reg0 = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg0},
			.write={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Test with too short request */
	uint8_t pdu_data_short[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x00, 0xFF, /* AND mask */
		0x00, /* Missing OR mask low byte */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data_short, sizeof pdu_data_short, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);

	/* Test with too long request */
	uint8_t pdu_data_long[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x00, 0xFF, /* AND mask */
		0x00, 0x00, /* OR mask */
		0x00, /* Extra byte */
	};

	res_size = mbpdu_handle_req(&inst, pdu_data_long, sizeof pdu_data_long, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_mask_write_nonexistent_addr_fails)
{
	uint16_t reg0 = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg0},
			.write={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Non-existent addr */
		0x00, 0xFF, /* AND mask */
		0x00, 0x00, /* OR mask */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_mask_write_readonly_reg_fails)
{
	uint16_t reg0 = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR, /* Read-only */
			.read={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x00, 0xFF, /* AND mask */
		0x00, 0x00, /* OR mask */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_mask_write_all_bits_set_works)
{
	uint16_t reg0 = 0x0000;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg0},
			.write={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x00, 0x00, /* AND mask - keep no bits */
		0xFF, 0xFF, /* OR mask - set all bits */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	ASSERT_EQ(0xFFFF, reg0); /* All bits should be set */
}

TEST(mbpdu_mask_write_all_bits_clear_works)
{
	uint16_t reg0 = 0xFFFF;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg0},
			.write={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x00, 0x00, /* AND mask - clear all bits */
		0x00, 0x00, /* OR mask - no bits set */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	ASSERT_EQ(0x0000, reg0); /* All bits should be cleared */
}

TEST(mbpdu_mask_write_selective_bits_works)
{
	uint16_t reg0 = 0b1010101010101010;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg0},
			.write={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0b11110000, 0b11110000, /* AND mask - preserve upper 4 bits of each byte */
		0b00001111, 0b00001111, /* OR mask - set lower 4 bits of each byte */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	/* Original: 1010101010101010
	 * AND:      1111000011110000 -> 1010000010100000
	 * OR:       0000111100001111 -> 1010111110101111
	 */
	ASSERT_EQ(0b1010111110101111, reg0);
}

TEST(mbpdu_mask_write_no_change_works)
{
	uint16_t reg0 = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg0},
			.write={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0xFF, 0xFF, /* AND mask - preserve all bits */
		0x00, 0x00, /* OR mask - set no bits */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	ASSERT_EQ(0x1234, reg0); /* Value should remain unchanged */
}

TEST(mbpdu_mask_write_high_address_works)
{
	uint16_t reg0 = 0x5555;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0xFFFF,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&reg0},
			.write={.pu16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0xFF, 0xFF, /* High address */
		0xAA, 0xAA, /* AND mask */
		0x11, 0x11, /* OR mask */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	ASSERT_EQ(0xFF, res[1]); /* Addr H */
	ASSERT_EQ(0xFF, res[2]); /* Addr L */
	ASSERT_EQ(0xAA, res[3]); /* AND mask H */
	ASSERT_EQ(0xAA, res[4]); /* AND mask L */
	ASSERT_EQ(0x11, res[5]); /* OR mask H */
	ASSERT_EQ(0x11, res[6]); /* OR mask L */
	/* 0x5555 & 0xAAAA = 0x0000, then 0x0000 | 0x1111 = 0x1111 */
	ASSERT_EQ(0x1111, reg0);
}

TEST(mbpdu_mask_write_u8_works)
{
	uint8_t reg0 = 0xAB;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U8,
			.access=MRACC_RW_PTR,
			.read={.pu8=&reg0},
			.write={.pu8=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x00, 0xF0, /* AND mask - preserve upper nibble */
		0x00, 0x05, /* OR mask - set bits 0 and 2 in lower nibble */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	/* 0xAB & 0xF0 = 0xA0, then 0xA0 | 0x05 = 0xA5 */
	ASSERT_EQ(0xA5, reg0);
}

TEST(mbpdu_mask_write_u32_works)
{
	uint32_t reg0 = 0x12345678;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U32,
			.access=MRACC_RW_PTR,
			.read={.pu32=&reg0},
			.write={.pu32=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Test writing to the lower 16 bits (register address 0x01) */
	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x01, /* Addr - lower 16 bits */
		0xFF, 0x00, /* AND mask - clear lower 8 bits */
		0x00, 0xFF, /* OR mask - set lower 8 bits to 0xFF */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	/* Original: 0x12345678, lower 16 bits: 0x5678
	 * 0x5678 & 0xFF00 = 0x5600, then 0x5600 | 0x00FF = 0x56FF
	 * Result: 0x123456FF */
	ASSERT_EQ(0x123456FF, reg0);
}

TEST(mbpdu_mask_write_i16_works)
{
	int16_t reg0 = -12345; /* 0xCFC7 */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_I16,
			.access=MRACC_RW_PTR,
			.read={.pi16=&reg0},
			.write={.pi16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x80, 0x00, /* AND mask - preserve only sign bit */
		0x00, 0x01, /* OR mask - set LSB */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	/* 0xCFC7 & 0x8000 = 0x8000, then 0x8000 | 0x0001 = 0x8001 */
	ASSERT_EQ((int16_t)0x8001, reg0); /* Still negative: -32767 */
}

TEST(mbpdu_mask_write_i32_works)
{
	int32_t reg0 = -0x12345678;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_I32,
			.access=MRACC_RW_PTR,
			.read={.pi32=&reg0},
			.write={.pi32=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* Test writing to the upper 16 bits (register address 0x00) */
	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr - upper 16 bits */
		0x00, 0xFF, /* AND mask - preserve lower 8 bits of upper word */
		0xFF, 0x00, /* OR mask - set upper 8 bits of upper word */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	/* Original: 0xEDCBA988 (two's complement of 0x12345678)
	 * Upper 16 bits: 0xEDCB
	 * 0xEDCB & 0x00FF = 0x00CB, then 0x00CB | 0xFF00 = 0xFFCB
	 * Result: 0xFFCBA988 */
	ASSERT_EQ((int32_t)0xFFCBA988, reg0);
}

TEST(mbpdu_mask_write_different_addresses_u32)
{
	uint32_t reg0 = 0xAABBCCDD;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x10u,
			.type=MRTYPE_U32,
			.access=MRACC_RW_PTR,
			.read={.pu32=&reg0},
			.write={.pu32=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	/* First test: mask write on upper 16 bits (address 0x10) */
	uint8_t pdu_data1[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x10, /* Addr - upper 16 bits */
		0x00, 0xFF, /* AND mask */
		0x11, 0x00, /* OR mask */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data1, sizeof pdu_data1, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	/* 0xAABB & 0x00FF = 0x00BB, then 0x00BB | 0x1100 = 0x11BB
	 * Result: 0x11BBCCDD */
	ASSERT_EQ(0x11BBCCDD, reg0);

	/* Second test: mask write on lower 16 bits (address 0x11) */
	uint8_t pdu_data2[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x11, /* Addr - lower 16 bits */
		0xFF, 0x00, /* AND mask */
		0x00, 0x22, /* OR mask */
	};

	res_size = mbpdu_handle_req(&inst, pdu_data2, sizeof pdu_data2, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	/* 0xCCDD & 0xFF00 = 0xCC00, then 0xCC00 | 0x0022 = 0xCC22
	 * Result: 0x11BBCC22 */
	ASSERT_EQ(0x11BBCC22, reg0);
}

TEST(mbpdu_mask_write_signed_negative_preservation)
{
	int16_t reg0 = -1; /* 0xFFFF */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_I16,
			.access=MRACC_RW_PTR,
			.read={.pi16=&reg0},
			.write={.pi16=&reg0},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0xFF, 0xFE, /* AND mask - clear LSB only */
		0x00, 0x00, /* OR mask - set nothing */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	/* 0xFFFF & 0xFFFE = 0xFFFE, then 0xFFFE | 0x0000 = 0xFFFE */
	ASSERT_EQ((int16_t)0xFFFE, reg0); /* -2 */
}

static uint16_t s_test_mask_write_fn_value = 0x5555;

static uint16_t test_mask_write_read_fn(void)
{
	return s_test_mask_write_fn_value;
}

static enum mbstatus_e test_mask_write_write_fn(uint16_t value)
{
	s_test_mask_write_fn_value = value;
	return MB_OK;
}

TEST(mbpdu_mask_write_function_access_works)
{
	s_test_mask_write_fn_value = 0xAAAA;

	const struct mbreg_desc_s regs[] = {
		{
			.address=0x00u,
			.type=MRTYPE_U16,
			.access=MRACC_RW_FN,
			.read={.fu16=test_mask_write_read_fn},
			.write={.fu16=test_mask_write_write_fn},
		},
	};
	struct mbinst_s inst = {
		.hold_regs=regs,
		.n_hold_regs=sizeof regs / sizeof regs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_MASK_WRITE_REG,
		0x00, 0x00, /* Addr */
		0x0F, 0x0F, /* AND mask - preserve lower nibbles */
		0x50, 0x50, /* OR mask - set specific pattern in upper nibbles */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(7u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_MASK_WRITE_REG, res[0]);
	/* 0xAAAA & 0x0F0F = 0x0A0A, then 0x0A0A | 0x5050 = 0x5A5A */
	ASSERT_EQ(0x5A5A, s_test_mask_write_fn_value);
}

TEST_MAIN(
	mbpdu_read_holding_reg_works,
	mbpdu_read_input_reg_works,
	mbpdu_illegal_fn_fails,
	mbpdu_write_single_reg_works,
	mbpdu_write_multiple_regs_works,
	mbpdu_read_nonexistent_holding_reg_fails,
	mbpdu_read_nonexistent_input_reg_fails,
	mbpdu_read_multiple_holding_regs_works,
	mbpdu_read_multiple_input_regs_works,
	mbpdu_write_to_readonly_reg_fails,
	mbpdu_write_nonexistent_reg_fails,
	mbpdu_read_with_invalid_quantity_fails,
	mbpdu_write_multiple_regs_invalid_byte_count_fails,
	mbpdu_null_inst_safe,
	mbpdu_empty_pdu_safe,
	mbpdu_read_partial_register_range_works,
	mbpdu_write_across_register_gap_fails,
	mbpdu_read_max_quantity_holding_regs,
	mbpdu_read_max_quantity_input_regs,
	mbpdu_read_excess_quantity_holding_regs_fails,
	mbpdu_read_excess_quantity_input_regs_fails,
	mbpdu_write_zero_quantity_fails,
	mbpdu_read_u32_reg_by_pointer,
	mbpdu_read_float_reg_by_pointer,
	mbpdu_read_reg_by_callback,
	mbpdu_write_reg_by_callback,
	mbpdu_read_locked_reg_access_denied,
	mbpdu_write_locked_reg_access_denied,
	mbpdu_post_write_callback_executed,
	mbpdu_inst_commit_callback_executed,
	mbpdu_block_u16_read_works,
	mbpdu_block_u16_read_middle_works,
	mbpdu_block_u32_read_works,
	mbpdu_block_read_beyond_end_fails,
	mbpdu_block_write_works,
	mbpdu_block_register_out_of_bounds,
	mbpdu_block_read_u8_works,
	mbpdu_block_write_u8_works,
	mbpdu_block_write_u8_upper_byte_ignored,
	mbpdu_block_read_i8_works,
	mbpdu_block_write_i8_works,
	mbpdu_custom_function_handler_works,
	mbpdu_custom_handler_fallback_for_standard_functions,
	mbpdu_word_order_swap_u32_works,
	mbpdu_word_order_swap_u64_works,
	mbpdu_mixed_register_types_work,
	mbpdu_read_holding_regs_invalid_quantity_zero,
	mbpdu_write_single_reg_invalid_address,
	mbpdu_write_multiple_regs_mismatched_byte_count,
	mbpdu_read_write_regs_basic_works,
	mbpdu_read_write_regs_multiple_works,
	mbpdu_read_write_regs_same_register_works,
	mbpdu_read_write_regs_invalid_request_length_fails,
	mbpdu_read_write_regs_byte_count_mismatch_fails,
	mbpdu_read_write_regs_nonexistent_read_addr_fails,
	mbpdu_read_write_regs_nonexistent_write_addr_fails,
	mbpdu_read_write_regs_write_readonly_fails,
	mbpdu_read_write_regs_commit_callback_executed,
	mbpdu_read_write_regs_zero_read_quantity_fails,
	mbpdu_read_write_regs_zero_write_quantity_fails,
	mbpdu_write_partial_reg_fc_cb_fails,
	mbpdu_mask_write_res_works,
	mbpdu_mask_write_invalid_request_length_fails,
	mbpdu_mask_write_nonexistent_addr_fails,
	mbpdu_mask_write_readonly_reg_fails,
	mbpdu_mask_write_all_bits_set_works,
	mbpdu_mask_write_all_bits_clear_works,
	mbpdu_mask_write_selective_bits_works,
	mbpdu_mask_write_no_change_works,
	mbpdu_mask_write_high_address_works,
	mbpdu_mask_write_u8_works,
	mbpdu_mask_write_u32_works,
	mbpdu_mask_write_i16_works,
	mbpdu_mask_write_i32_works,
	mbpdu_mask_write_different_addresses_u32,
	mbpdu_mask_write_signed_negative_preservation,
	mbpdu_mask_write_function_access_works
);
