#include "test_lib.h"
#include <mbcoil.h>
#include <mbdef.h>
#include <mbfn_coils.h>
#include <mbinst.h>
#include <mbpdu.h>
#include <stdint.h>

/* --- Helpers --- */

static int s_always_locked_called = 0;
static int always_locked(void) { ++s_always_locked_called; return 1; }

static int s_post_write_count = 0;
static void post_write_cb(void) { ++s_post_write_count; }

static int s_commit_count = 0;
static void commit_coils_cb(const struct mbinst_s *inst) { (void)inst; ++s_commit_count; }

static uint8_t s_coil_byte = 0x00u;

static enum mbstatus_e write_fn_ok(int value) { s_coil_byte = (uint8_t)value; return MB_OK; }

/* --- mbfn_read_coils: null and invalid parameter checks --- */

TEST(mbfn_read_coils_null_inst_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	uint8_t req[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_coils(NULL, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_coils_null_coils_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_coils(&inst, NULL, 0u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_coils_null_req_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_coils(&inst, coils, 1u, NULL, 5u, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_coils_null_res_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};

	enum mbstatus_e status = mbfn_read_coils(&inst, coils, 1u, req, sizeof req, NULL);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_coils_wrong_fc_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_HOLDING_REGS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_coils(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_coils_wrong_req_len_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_coils(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

TEST(mbfn_read_coils_zero_quantity_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x00u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_coils(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

/* --- mbfn_read_coils: integration via mbpdu_handle_req --- */

TEST(mbpdu_read_single_coil_on_works)
{
	uint8_t coil_byte = 0x01u; /* bit 0 set = ON */
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(3u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_COILS, res[0]);
	ASSERT_EQ(1u, res[1]); /* byte count */
	ASSERT_EQ(0x01u, res[2]); /* coil 0 is ON, bit 0 set */
}

TEST(mbpdu_read_single_coil_off_works)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(3u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(1u, res[1]);
	ASSERT_EQ(0x00u, res[2]);
}

TEST(mbpdu_read_coils_byte_count_is_ceil_quantity_over_8)
{
	uint8_t coil_byte = 0xFFu;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}},
		{.address=0x0001u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=1u}},
		{.address=0x0002u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=2u}},
		{.address=0x0003u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=3u}},
		{.address=0x0004u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=4u}},
		{.address=0x0005u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=5u}},
		{.address=0x0006u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=6u}},
		{.address=0x0007u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=7u}},
		{.address=0x0008u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}},
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* 9 coils → (9+7)/8 = 2 bytes */
	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x09u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(4u, res_size); /* 1 fc + 1 byte_count + 2 data bytes */
	ASSERT_EQ(2u, res[1]);   /* byte count = 2 */
}

TEST(mbpdu_read_coils_packs_bits_lsb_first)
{
	uint8_t coil_byte = 0b00000101u; /* bits 0 and 2 set */
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}, /* ON */
		{.address=0x0001u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=1u}}, /* OFF */
		{.address=0x0002u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=2u}}, /* ON */
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* Read 3 coils starting at 0x0000 */
	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x03u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(1u, res[1]); /* byte count */
	/* coil 0 = bit 0, coil 1 = bit 1, coil 2 = bit 2: 0b00000101 = 0x05 */
	ASSERT_EQ(0x05u, res[2]);
}

TEST(mbpdu_read_coils_first_addr_missing_fails)
{
	uint8_t coil_byte = 0x01u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0010u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_read_coils_missing_middle_coil_is_zero)
{
	/* coil 0 exists, coil 1 does not, coil 2 exists */
	uint8_t byte_a = 0x01u;
	uint8_t byte_b = 0x01u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&byte_a, .ix=0u}}, /* ON */
		{.address=0x0002u, .access=MCACC_R_PTR, .read={.ptr=&byte_b, .ix=0u}}, /* ON */
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* Read 3 coils; coil 1 is absent → zero */
	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x03u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	/* coil 0=ON(bit0), coil 1=0(bit1), coil 2=ON(bit2): 0b00000101 = 0x05 */
	ASSERT_EQ(0x05u, res[2]);
}

TEST(mbpdu_read_coils_locked_fails)
{
	s_always_locked_called = 0;
	uint8_t coil_byte = 0x01u;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000u,
			.access=MCACC_R_PTR,
			.read={.ptr=&coil_byte, .ix=0u},
			.rlock_cb=always_locked
		}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
	ASSERT(s_always_locked_called > 0);
}

TEST(mbpdu_read_disc_inputs_works)
{
	uint8_t coil_byte = 0x01u; /* bit 0 = ON */
	const struct mbcoil_desc_s inputs[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.disc_inputs=inputs,
		.n_disc_inputs=sizeof inputs / sizeof inputs[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_DISC_INPUTS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(3u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_DISC_INPUTS, res[0]);
	ASSERT_EQ(1u, res[1]);
	ASSERT_EQ(0x01u, res[2]);
}

/* --- mbfn_write_coil: null and invalid parameter checks --- */

TEST(mbfn_write_coil_null_inst_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	uint8_t req[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu, 0x00u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coil(NULL, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_write_coil_null_coils_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu, 0x00u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coil(&inst, NULL, 0u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_write_coil_null_req_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coil(&inst, coils, 1u, NULL, 5u, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_write_coil_null_res_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu, 0x00u};

	enum mbstatus_e status = mbfn_write_coil(&inst, coils, 1u, req, sizeof req, NULL);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_write_coil_wrong_fc_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0xFFu, 0x00u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coil(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_write_coil_wrong_req_len_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coil(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

TEST(mbfn_write_coil_invalid_value_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	/* Coil value 0x1234 is not valid (must be 0x0000 or 0xFF00) */
	uint8_t req[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0x12u, 0x34u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coil(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

/* --- mbfn_write_coil: integration via mbpdu_handle_req --- */

TEST(mbpdu_write_single_coil_on_works)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0005u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x05u, 0xFFu, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(MBFC_WRITE_SINGLE_COIL, res[0]);
	ASSERT_EQ(1u, (coil_byte >> 0u) & 0x01u); /* bit 0 must be set */
}

TEST(mbpdu_write_single_coil_off_works)
{
	uint8_t coil_byte = 0x01u; /* Start ON */
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0x00u, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0u, coil_byte & 0x01u); /* bit 0 must be clear */
}

TEST(mbpdu_write_single_coil_echoes_request)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0042u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x42u, 0xFFu, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	/* Response echoes the request (address + value) */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x00u, res[1]); /* addr H */
	ASSERT_EQ(0x42u, res[2]); /* addr L */
	ASSERT_EQ(0xFFu, res[3]); /* val H */
	ASSERT_EQ(0x00u, res[4]); /* val L */
}

TEST(mbpdu_write_single_coil_addr_missing_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0010u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_write_single_coil_readonly_fails)
{
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_VAL, .read={.val=1u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_write_single_coil_calls_post_write_cb)
{
	s_post_write_count = 0;
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000u,
			.access=MCACC_RW_PTR,
			.read={.ptr=&coil_byte, .ix=0u},
			.write={.ptr=&coil_byte, .ix=0u},
			.post_write_cb=post_write_cb
		}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(1, s_post_write_count);
}

TEST(mbpdu_write_single_coil_calls_commit_cb)
{
	s_commit_count = 0;
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0],
		.commit_coils_write_cb=commit_coils_cb
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(1, s_commit_count);
}

/* --- mbfn_write_coils: null and invalid parameter checks --- */

TEST(mbfn_write_coils_null_inst_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	/* qty=1, byte_count=1, data=0x01 */
	uint8_t req[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x01u, 0x01u, 0x01u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coils(NULL, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_write_coils_null_coils_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x01u, 0x01u, 0x01u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coils(&inst, NULL, 0u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_write_coils_wrong_fc_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u, 0x01u, 0x01u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coils(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_write_coils_too_short_req_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	/* Minimum valid length is 7; send only 6 */
	uint8_t req[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x01u, 0x01u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coils(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

TEST(mbfn_write_coils_zero_quantity_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	/* qty=0 → illegal data value */
	uint8_t req[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coils(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

TEST(mbfn_write_coils_byte_count_mismatch_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	/* qty=1 requires byte_count=1, but claim byte_count=2 */
	uint8_t req[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x01u, 0x00u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_write_coils(&inst, coils, 1u, req, sizeof req, &res);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

/* --- mbfn_write_coils: integration via mbpdu_handle_req --- */

TEST(mbpdu_write_multiple_coils_single_works)
{
	s_coil_byte = 0u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_W_FN, .write={.fn=write_fn_ok}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* qty=1, byte_count=1, data=ON(bit0=1) */
	uint8_t pdu[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x01u, 0x01u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(MBFC_WRITE_MULTIPLE_COILS, res[0]);
	ASSERT_EQ(1u, s_coil_byte); /* callback received ON */
}

TEST(mbpdu_write_multiple_coils_response_contains_addr_and_qty)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0020u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x20u, 0x00u, 0x01u, 0x01u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x00u, res[1]); /* start addr H */
	ASSERT_EQ(0x20u, res[2]); /* start addr L */
	ASSERT_EQ(0x00u, res[3]); /* quantity H */
	ASSERT_EQ(0x01u, res[4]); /* quantity L */
}

TEST(mbpdu_write_multiple_coils_bit_order_correct)
{
	uint8_t bytes[3] = {0u, 0u, 0u};
	/* 3 coils each mapped to bit 0 of a separate byte */
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&bytes[0], .ix=0u}, .write={.ptr=&bytes[0], .ix=0u}},
		{.address=0x0001u, .access=MCACC_RW_PTR, .read={.ptr=&bytes[1], .ix=0u}, .write={.ptr=&bytes[1], .ix=0u}},
		{.address=0x0002u, .access=MCACC_RW_PTR, .read={.ptr=&bytes[2], .ix=0u}, .write={.ptr=&bytes[2], .ix=0u}},
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* Write ON=1, OFF=0, ON=1: data byte = 0b00000101 = 0x05 */
	uint8_t pdu[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x03u, 0x01u, 0x05u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(1u, bytes[0] & 0x01u); /* coil 0 ON */
	ASSERT_EQ(0u, bytes[1] & 0x01u); /* coil 1 OFF */
	ASSERT_EQ(1u, bytes[2] & 0x01u); /* coil 2 ON */
}

TEST(mbpdu_write_multiple_coils_addr_missing_fails)
{
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0010u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte, .ix=0u}, .write={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x01u, 0x01u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_write_multiple_coils_readonly_fails)
{
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_VAL, .read={.val=1u}}
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x01u, 0x01u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_write_multiple_coils_calls_post_write_cb_per_coil)
{
	s_post_write_count = 0;
	uint8_t bytes[2] = {0u, 0u};
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0000u,
			.access=MCACC_RW_PTR,
			.read={.ptr=&bytes[0], .ix=0u},
			.write={.ptr=&bytes[0], .ix=0u},
			.post_write_cb=post_write_cb
		},
		{
			.address=0x0001u,
			.access=MCACC_RW_PTR,
			.read={.ptr=&bytes[1], .ix=0u},
			.write={.ptr=&bytes[1], .ix=0u},
			.post_write_cb=post_write_cb
		},
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0]
	};
	mbinst_init(&inst);

	/* Write 2 coils */
	uint8_t pdu[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x02u, 0x01u, 0x03u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(2, s_post_write_count); /* one call per coil */
}

TEST(mbpdu_write_multiple_coils_calls_commit_cb_once)
{
	s_commit_count = 0;
	uint8_t bytes[2] = {0u, 0u};
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_RW_PTR, .read={.ptr=&bytes[0], .ix=0u}, .write={.ptr=&bytes[0], .ix=0u}},
		{.address=0x0001u, .access=MCACC_RW_PTR, .read={.ptr=&bytes[1], .ix=0u}, .write={.ptr=&bytes[1], .ix=0u}},
	};
	struct mbinst_s inst = {
		.coils=coils,
		.n_coils=sizeof coils / sizeof coils[0],
		.commit_coils_write_cb=commit_coils_cb
	};
	mbinst_init(&inst);

	/* Write 2 coils in one request */
	uint8_t pdu[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x02u, 0x01u, 0x03u};
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(1, s_commit_count); /* called exactly once regardless of coil count */
}

TEST_MAIN(
	mbfn_read_coils_null_inst_fails,
	mbfn_read_coils_null_coils_fails,
	mbfn_read_coils_null_req_fails,
	mbfn_read_coils_null_res_fails,
	mbfn_read_coils_wrong_fc_fails,
	mbfn_read_coils_wrong_req_len_fails,
	mbfn_read_coils_zero_quantity_fails,
	mbpdu_read_single_coil_on_works,
	mbpdu_read_single_coil_off_works,
	mbpdu_read_coils_byte_count_is_ceil_quantity_over_8,
	mbpdu_read_coils_packs_bits_lsb_first,
	mbpdu_read_coils_first_addr_missing_fails,
	mbpdu_read_coils_missing_middle_coil_is_zero,
	mbpdu_read_coils_locked_fails,
	mbpdu_read_disc_inputs_works,
	mbfn_write_coil_null_inst_fails,
	mbfn_write_coil_null_coils_fails,
	mbfn_write_coil_null_req_fails,
	mbfn_write_coil_null_res_fails,
	mbfn_write_coil_wrong_fc_fails,
	mbfn_write_coil_wrong_req_len_fails,
	mbfn_write_coil_invalid_value_fails,
	mbpdu_write_single_coil_on_works,
	mbpdu_write_single_coil_off_works,
	mbpdu_write_single_coil_echoes_request,
	mbpdu_write_single_coil_addr_missing_fails,
	mbpdu_write_single_coil_readonly_fails,
	mbpdu_write_single_coil_calls_post_write_cb,
	mbpdu_write_single_coil_calls_commit_cb,
	mbfn_write_coils_null_inst_fails,
	mbfn_write_coils_null_coils_fails,
	mbfn_write_coils_wrong_fc_fails,
	mbfn_write_coils_too_short_req_fails,
	mbfn_write_coils_zero_quantity_fails,
	mbfn_write_coils_byte_count_mismatch_fails,
	mbpdu_write_multiple_coils_single_works,
	mbpdu_write_multiple_coils_response_contains_addr_and_qty,
	mbpdu_write_multiple_coils_bit_order_correct,
	mbpdu_write_multiple_coils_addr_missing_fails,
	mbpdu_write_multiple_coils_readonly_fails,
	mbpdu_write_multiple_coils_calls_post_write_cb_per_coil,
	mbpdu_write_multiple_coils_calls_commit_cb_once
);
