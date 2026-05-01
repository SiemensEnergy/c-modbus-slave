#include "test_lib.h"
#include <mbdef.h>
#include <mbfn_serial.h>
#include <mbinst.h>
#include <mbpdu.h>
#include <stdint.h>

/* --- Helpers --- */

static uint8_t s_exception_status = 0x00u;
static uint8_t read_exception_status_cb(void) { return s_exception_status; }

/* --- mbfn_read_exception_status: null and invalid parameter checks --- */

TEST(mbfn_read_exception_status_null_inst_fails)
{
	uint8_t req[] = {MBFC_READ_EXCEPTION_STATUS};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_exception_status(NULL, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_exception_status_null_req_fails)
{
	struct mbinst_s inst = {
		.serial = {.read_exception_status_cb=read_exception_status_cb}
	};
	mbinst_init(&inst);
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_exception_status(&inst, NULL, 1u, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_exception_status_null_res_fails)
{
	struct mbinst_s inst = {
		.serial = {.read_exception_status_cb=read_exception_status_cb}
	};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_EXCEPTION_STATUS};

	enum mbstatus_e status = mbfn_read_exception_status(&inst, req, sizeof req, NULL);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_exception_status_null_cb_fails)
{
	struct mbinst_s inst = {0}; /* No callback configured */
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_EXCEPTION_STATUS};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_exception_status(&inst, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_exception_status_wrong_fc_fails)
{
	struct mbinst_s inst = {
		.serial = {.read_exception_status_cb=read_exception_status_cb}
	};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_COILS}; /* Wrong function code */
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_exception_status(&inst, req, sizeof req, &res);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbfn_read_exception_status_wrong_req_len_fails)
{
	struct mbinst_s inst = {
		.serial = {.read_exception_status_cb=read_exception_status_cb}
	};
	mbinst_init(&inst);
	/* Length must be exactly 1 (just the function code); send 2 */
	uint8_t req[] = {MBFC_READ_EXCEPTION_STATUS, 0x00u};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_exception_status(&inst, req, sizeof req, &res);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, status);
}

TEST(mbfn_read_exception_status_returns_cb_value)
{
	s_exception_status = 0xA5u;
	struct mbinst_s inst = {
		.serial = {.read_exception_status_cb=read_exception_status_cb}
	};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_EXCEPTION_STATUS};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_exception_status(&inst, req, sizeof req, &res);

	ASSERT_EQ(MB_OK, status);
	ASSERT_EQ(2u, res.size);
	ASSERT_EQ(0xA5u, res_buf[1]); /* exception status byte */
}

TEST(mbfn_read_exception_status_zero_value_works)
{
	s_exception_status = 0x00u;
	struct mbinst_s inst = {
		.serial = {.read_exception_status_cb=read_exception_status_cb}
	};
	mbinst_init(&inst);
	uint8_t req[] = {MBFC_READ_EXCEPTION_STATUS};
	uint8_t res_buf[MBPDU_SIZE_MAX];
	struct mbpdu_buf_s res = {.p=res_buf, .size=0u};

	enum mbstatus_e status = mbfn_read_exception_status(&inst, req, sizeof req, &res);

	ASSERT_EQ(MB_OK, status);
	ASSERT_EQ(2u, res.size);
	ASSERT_EQ(0x00u, res_buf[1]);
}

TEST_MAIN(
	mbfn_read_exception_status_null_inst_fails,
	mbfn_read_exception_status_null_req_fails,
	mbfn_read_exception_status_null_res_fails,
	mbfn_read_exception_status_null_cb_fails,
	mbfn_read_exception_status_wrong_fc_fails,
	mbfn_read_exception_status_wrong_req_len_fails,
	mbfn_read_exception_status_returns_cb_value,
	mbfn_read_exception_status_zero_value_works
);
