#include "test_lib.h"
#include <mbinst.h>
#include <mbcoil.h>
#include <mbpdu.h>
#include <stdint.h>

TEST(mbcoil_null_coil_read_fails)
{
	int result = mbcoil_read(NULL);
	ASSERT_EQ(MBCOIL_READ_DEV_FAIL, result);
}

TEST(mbcoil_null_coil_write_fails)
{
	enum mbstatus_e status = mbcoil_write(NULL, 1);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbcoil_null_coil_write_allowed_fails)
{
	int result = mbcoil_write_allowed(NULL);
	ASSERT_EQ(0, result);
}

TEST(mbcoil_invalid_access_read_fails)
{
	uint8_t test_val = 0x55;
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_W_PTR, /* Write-only, no read access */
		.write={.ptr=&test_val}
	};

	int val = mbcoil_read(&coil);
	ASSERT_EQ(MBCOIL_READ_NO_ACCESS, val);
}

TEST(mbcoil_invalid_access_write_fails)
{
	uint8_t test_val = 0x55;
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_R_PTR, /* Read-only, no write access */
		.read={.ptr=&test_val}
	};

	enum mbstatus_e status = mbcoil_write(&coil, 1);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbcoil_null_ptr_read_fails)
{
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_R_PTR,
		.read={.ptr=NULL}
	};

	int val = mbcoil_read(&coil);
	ASSERT_EQ(MBCOIL_READ_DEV_FAIL, val);
}

TEST(mbcoil_null_ptr_write_fails)
{
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_W_PTR,
		.write={.ptr=NULL}
	};

	enum mbstatus_e status = mbcoil_write(&coil, 1);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbcoil_null_fn_read_fails)
{
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_R_FN,
		.read={.fn=NULL}
	};

	int val = mbcoil_read(&coil);
	ASSERT_EQ(MBCOIL_READ_DEV_FAIL, val);
}

TEST(mbcoil_null_fn_write_fails)
{
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_W_FN,
		.write={.fn=NULL}
	};

	enum mbstatus_e status = mbcoil_write(&coil, 1);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

static int always_locked(void)
{
	return 1; /* Always locked */
}
TEST(mbcoil_read_locked_fails)
{
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_R_VAL,
		.read={.val=1},
		.rlock_cb=always_locked
	};

	int val = mbcoil_read(&coil);
	ASSERT_EQ(MBCOIL_READ_LOCKED, val);
}

TEST(mbcoil_write_locked_fails)
{
	uint8_t test_val = 0u;
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_W_PTR,
		.write={.ptr=&test_val},
		.wlock_cb=always_locked
	};

	int allowed = mbcoil_write_allowed(&coil);
	ASSERT_EQ(0, allowed);
}

static enum mbstatus_e failing_write_fn(uint8_t value)
{
	(void)value;
	return MB_ILLEGAL_DATA_VAL; /* Always fail */
}
TEST(mbcoil_function_write_failure)
{
	const struct mbcoil_desc_s coil = {
		.address=0x0000,
		.access=MCACC_W_FN,
		.write={.fn=failing_write_fn}
	};

	enum mbstatus_e status = mbcoil_write(&coil, 1);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

TEST(mbcoil_find_null_array_fails)
{
	const struct mbcoil_desc_s *desc = mbcoil_find_desc(NULL, 10, 0x0000);
	ASSERT_EQ(NULL, desc);
}

TEST(mbcoil_find_zero_count_fails)
{
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000,
			.access=MCACC_R_VAL,
			.read={.val=0},
		}
	};

	const struct mbcoil_desc_s *desc = mbcoil_find_desc(coils, 0, 0x0000);
	ASSERT_EQ(NULL, desc);
}

TEST(mbcoil_find_nonexistent_address_fails)
{
	uint8_t test_val = 0x55;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000,
			.access=MCACC_RW_PTR,
			.read={.ptr=&test_val, .ix=0},
			.write={.ptr=&test_val, .ix=0}
		},
		{
			.address=0x0002,
			.access=MCACC_RW_PTR,
			.read={.ptr=&test_val, .ix=1},
			.write={.ptr=&test_val, .ix=1}
		}
	};

	const struct mbcoil_desc_s *desc = mbcoil_find_desc(
		coils, sizeof coils / sizeof coils[0], 0x0001);
	ASSERT_EQ(NULL, desc);
}

TEST(mbpdu_read_coils_invalid_address_range)
{
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000,
			.access=MCACC_R_VAL,
			.read={.val=1}
		}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* Try to read from address 0x1000 which doesn't exist */
	uint8_t pdu_data[] = {
		MBFC_READ_COILS,
		0x10, 0x00, /* Start addr 0x1000 */
		0x00, 0x01, /* Quantity: 1 coil */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_write_single_coil_invalid_address)
{
	uint8_t test_val = 0x55;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000,
			.access=MCACC_RW_PTR,
			.read={.ptr=&test_val},
			.write={.ptr=&test_val}
		}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* Try to write to address 0x1000 which doesn't exist */
	uint8_t pdu_data[] = {
		MBFC_WRITE_SINGLE_COIL,
		0x10, 0x00, /* Address 0x1000 */
		0xFF, 0x00, /* Value: ON */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);

	ASSERT_EQ(0x55, test_val); /* No values have been changed */
}

TEST(mbpdu_write_multiple_coils_invalid_byte_count)
{
	uint8_t test_val = 0x55;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000,
			.access=MCACC_RW_PTR,
			.read={.ptr=&test_val, .ix=0},
			.write={.ptr=&test_val, .ix=0}
		},
		{
			.address=0x0001,
			.access=MCACC_RW_PTR,
			.read={.ptr=&test_val, .ix=1},
			.write={.ptr=&test_val, .ix=1}
		}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* Malformed request with wrong byte count */
	uint8_t pdu_data[] = {
		MBFC_WRITE_MULTIPLE_COILS,
		0x00, 0x00, /* Start addr */
		0x00, 0x02, /* Quantity: 8 coils (should need 1 byte) */
		0x02,       /* Byte count: 2 (wrong, should be 1) */
		0xFF, 0xFF  /* Data bytes */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);

	ASSERT_EQ(0x55, test_val); /* No values have been changed */
}

TEST(mbpdu_read_coils_excessive_quantity)
{
	uint8_t test_val = 0x55;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000,
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val}
		}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* Try to read too many coils (>2000) */
	uint8_t pdu_data[] = {
		MBFC_READ_COILS,
		0x00, 0x00, /* Start addr */
		0x08, 0x00, /* Quantity: 2048 coils (too many) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbcoil_read_coils)
{
	uint8_t s_coil1 = 1;
	uint8_t s_coil3n5 = 2;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0,
			.access=MCACC_R_VAL,
			.read={.val=0},
		},
		{
			.address=1,
			.access=MCACC_R_PTR,
			.read={.ptr=&s_coil1},
		},
		{
			.address=3,
			.access=MCACC_R_PTR,
			.read={.ptr=&s_coil3n5, .ix=0},
		},
		{
			.address=5,
			.access=MCACC_R_PTR,
			.read={.ptr=&s_coil3n5, .ix=1},
		}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t req[] = {
		MBFC_READ_COILS,
		0x00, 0x00, /* Start addr */
		0x00, 0x08, /* n coils */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, req, sizeof req, res);

	ASSERT_EQ(MBFC_READ_COILS, res[0]);
	ASSERT_EQ(1u, res[1]); /* Byte count */
	ASSERT_EQ(3u, res_size);

	/* Check coil values: bit 0=1, bit 1=0, bits 2-4=0, bit 5=1, bits 6-7=0 */
	/* Expected: 0b00100001 = 0x21 */
	ASSERT_EQ(0b00100010, res[2]);
}

TEST(mbcoil_write_single_coil_on)
{
	uint8_t s_coil1 = 0;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=1,
			.access=MCACC_W_PTR,
			.write={.ptr=&s_coil1},
		},
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t req[] = {
		MBFC_WRITE_SINGLE_COIL,
		0x00, 0x01, /* Coil addr 1*/
		0xFF, 0x00}; /* Coil ON */

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, req, sizeof req, res);

	/* Check response */
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(MBFC_WRITE_SINGLE_COIL, res[0]);
	ASSERT_EQ(0x00u, res[1]); /* Address H */
	ASSERT_EQ(0x01u, res[2]); /* Address L */
	ASSERT_EQ(0xFFu, res[3]); /* Value H */
	ASSERT_EQ(0x00u, res[4]); /* Value L */

	/* Check actual coil value */
	ASSERT_EQ(0x01u, s_coil1);
}

TEST(mbcoil_write_single_coil_off)
{
	uint8_t s_coil1 = 0x01;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=1,
			.access=MCACC_W_PTR,
			.write={.ptr=&s_coil1},
		},
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t req[] = {
		MBFC_WRITE_SINGLE_COIL,
		0x00, 0x01, /* Coil addr 1*/
		0x00, 0x00}; /* Coil OFF */

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, req, sizeof req, res);

	/* Check response */
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(MBFC_WRITE_SINGLE_COIL, res[0]);
	ASSERT_EQ(0x00u, res[1]); /* Address H */
	ASSERT_EQ(0x01u, res[2]); /* Address L */
	ASSERT_EQ(0x00u, res[3]); /* Value H */
	ASSERT_EQ(0x00u, res[4]); /* Value L */

	/* Check actual coil value */
	ASSERT_EQ(0x00u, s_coil1);
}

TEST(mbcoil_write_multiple_coils)
{
	uint8_t s_coil0 = 0;
	uint8_t s_coil1 = 1;
	uint8_t s_coil2n3 = 2;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0,
			.access=MCACC_W_PTR,
			.write={.ptr=&s_coil0},
		},
		{
			.address=1,
			.access=MCACC_W_PTR,
			.write={.ptr=&s_coil1},
		},
		{
			.address=2,
			.access=MCACC_W_PTR,
			.write={.ptr=&s_coil2n3, .ix=0},
		},
		{
			.address=3,
			.access=MCACC_W_PTR,
			.write={.ptr=&s_coil2n3, .ix=1},
		}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t req[] = {
		MBFC_WRITE_MULTIPLE_COILS,
		0x00, 0x00, /* Start addr */
		0x00, 0x03, /* n coils */
		0x01, /* Byte count */
		0b0101 /* Data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, req, sizeof req, res);

	/* Check response */
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(MBFC_WRITE_MULTIPLE_COILS, res[0]);
	ASSERT_EQ(0x00u, res[1]); /* Start address H */
	ASSERT_EQ(0x00u, res[2]); /* Start address L */
	ASSERT_EQ(0x00u, res[3]); /* Quantity H */
	ASSERT_EQ(0x03u, res[4]); /* Quantity L */

	/* Check actual coil values */
	ASSERT_EQ(1u, s_coil0);
	ASSERT_EQ(0u, s_coil1);
	ASSERT_EQ(3u, s_coil2n3);
}

TEST(mbcoil_invalid_coil_address)
{
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0,
			.access=MCACC_R_VAL,
			.read={.val=0},
		},
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t req[] = {
		MBFC_READ_COILS,
		0x00, 0x10, /* Start addr (Does not exist) */
		0x00, 0x01, /* n coils */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, req, sizeof req, res);

	/* Check error response */
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ((MBFC_READ_COILS | 0x80), res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST_MAIN(
	mbcoil_null_coil_read_fails,
	mbcoil_null_coil_write_fails,
	mbcoil_null_coil_write_allowed_fails,
	mbcoil_invalid_access_read_fails,
	mbcoil_invalid_access_write_fails,
	mbcoil_null_ptr_read_fails,
	mbcoil_null_ptr_write_fails,
	mbcoil_null_fn_read_fails,
	mbcoil_null_fn_write_fails,
	mbcoil_read_locked_fails,
	mbcoil_write_locked_fails,
	mbcoil_function_write_failure,
	mbcoil_find_null_array_fails,
	mbcoil_find_zero_count_fails,
	mbcoil_find_nonexistent_address_fails,
	mbpdu_read_coils_invalid_address_range,
	mbpdu_write_single_coil_invalid_address,
	mbpdu_write_multiple_coils_invalid_byte_count,
	mbpdu_read_coils_excessive_quantity,
	mbcoil_read_coils,
	mbcoil_write_single_coil_on,
	mbcoil_write_single_coil_off,
	mbcoil_write_multiple_coils,
	mbcoil_invalid_coil_address
);
