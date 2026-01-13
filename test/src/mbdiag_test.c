#include "test_lib.h"
#include <mbinst.h>
#include <mbpdu.h>
#include <mbdef.h>

/* Test diagnostic function code 0x08 (MBFC_DIAGNOSTICS) with various subfunctions */

TEST(mbdiag_loopback_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Sub-function 0x00: Return Query Data (Loopback) */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x00, /* Sub-function: Loopback */
		0x12, 0x34, /* Test data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_DIAGNOSTICS, res[0]);
	ASSERT_EQ(0x00, res[1]); /* Sub-function H */
	ASSERT_EQ(0x00, res[2]); /* Sub-function L */
	ASSERT_EQ(0x12, res[3]); /* Echo data H */
	ASSERT_EQ(0x34, res[4]); /* Echo data L */
}

TEST(mbdiag_restart_comms_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	inst.state.comm_event_counter = 10u;
	inst.state.bus_msg_counter = 1u;
	inst.state.bus_comm_err_counter = 2u;
	inst.state.exception_counter = 3u;
	inst.state.msg_counter = 4u;
	inst.state.no_resp_counter = 5u;
	inst.state.nak_counter = 6u;
	inst.state.busy_counter = 7u;
	inst.state.bus_char_overrun_counter = 8u;

	/* Sub-function 0x01: Restart Communications Option */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x01, /* Sub-function: Restart Communications */
		0x00, 0x00, /* Data (should be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_DIAGNOSTICS, res[0]);
	ASSERT_EQ(0x00, res[1]); /* Sub-function H */
	ASSERT_EQ(0x01, res[2]); /* Sub-function L */
	ASSERT_EQ(0x00, res[3]); /* Data H */
	ASSERT_EQ(0x00, res[4]); /* Data L */

	ASSERT_EQ(0u, inst.state.comm_event_counter);
	ASSERT_EQ(0u, inst.state.bus_msg_counter);
	ASSERT_EQ(0u, inst.state.bus_comm_err_counter);
	ASSERT_EQ(0u, inst.state.exception_counter);
	ASSERT_EQ(0u, inst.state.msg_counter);
	ASSERT_EQ(0u, inst.state.no_resp_counter);
	ASSERT_EQ(0u, inst.state.nak_counter);
	ASSERT_EQ(0u, inst.state.busy_counter);
	ASSERT_EQ(0u, inst.state.bus_char_overrun_counter);
}

TEST(mbdiag_restart_comms_in_listen_only_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	inst.state.comm_event_counter = 10u;
	inst.state.bus_msg_counter = 1u;
	inst.state.bus_comm_err_counter = 2u;
	inst.state.exception_counter = 3u;
	inst.state.msg_counter = 4u;
	inst.state.no_resp_counter = 5u;
	inst.state.nak_counter = 6u;
	inst.state.busy_counter = 7u;
	inst.state.bus_char_overrun_counter = 8u;

	inst.state.is_listen_only = 1;

	/* Sub-function 0x01: Restart Communications Option */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x01, /* Sub-function: Restart Communications */
		0x00, 0x00, /* Data (should be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(0u, res_size);

	ASSERT_EQ(0u, inst.state.comm_event_counter);
	ASSERT_EQ(0u, inst.state.bus_msg_counter);
	ASSERT_EQ(0u, inst.state.bus_comm_err_counter);
	ASSERT_EQ(0u, inst.state.exception_counter);
	ASSERT_EQ(0u, inst.state.msg_counter);
	ASSERT_EQ(0u, inst.state.no_resp_counter);
	ASSERT_EQ(0u, inst.state.nak_counter);
	ASSERT_EQ(0u, inst.state.busy_counter);
	ASSERT_EQ(0u, inst.state.bus_char_overrun_counter);
}

static int s_restart_called = 0;
static void test_restart_callback(void) {
	s_restart_called = 1;
}
TEST(mbdiag_restart_comms_with_callback)
{
	struct mbinst_s inst = {
		.serial = {
			.request_restart = test_restart_callback
		}
	};
	mbinst_init(&inst);
	s_restart_called = 0;
	inst.state.event_log_write_pos = 1;
	inst.state.event_log_count = 1;

	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x01, /* Sub-function: Restart Communications */
		0xFF, 0x00, /* Data: Clear event log and restart */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0xFF, res[3]); /* Echo data H */
	ASSERT_EQ(0x00, res[4]); /* Echo data L */

	ASSERT_EQ(1, s_restart_called); /* Callback should have be invoked */
	ASSERT_EQ(1, inst.state.event_log_write_pos);
	ASSERT_EQ(1, inst.state.event_log_count);
}

TEST(mbdiag_restart_comms_invalid_data_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	inst.state.busy_counter = 123;

	/* Invalid data value for restart comms */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x01, /* Sub-function: Restart Communications */
		0x12, 0x34, /* Invalid data (not 0x0000 or 0xFF00) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
	ASSERT_EQ(123, inst.state.busy_counter); /* Not modifed */
}

static uint16_t s_test_diag_value = 0x5678;
static uint16_t test_read_diagnostics_callback(void) {
	return s_test_diag_value;
}
TEST(mbdiag_read_diagnostic_register_works)
{
	struct mbinst_s inst = {
		.serial = {
			.read_diagnostics_cb = test_read_diagnostics_callback
		}
	};
	mbinst_init(&inst);

	/* Sub-function 0x02: Return Diagnostic Register */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x02, /* Sub-function: Read Diagnostic Register */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_DIAGNOSTICS, res[0]);
	ASSERT_EQ(0x00, res[1]); /* Sub-function H */
	ASSERT_EQ(0x02, res[2]); /* Sub-function L */
	ASSERT_EQ(0x56, res[3]); /* Diagnostic register H */
	ASSERT_EQ(0x78, res[4]); /* Diagnostic register L */
}

TEST(mbdiag_read_diagnostic_register_no_callback)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x02, /* Sub-function: Read Diagnostic Register */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x00, res[3]); /* Should return 0 when no callback */
	ASSERT_EQ(0x00, res[4]);
}

TEST(mbdiag_read_diagnostic_register_invalid_data_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x02, /* Sub-function: Read Diagnostic Register */
		0x12, 0x34, /* Invalid data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbdiag_change_ascii_delimiter_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Sub-function 0x03: Change ASCII Input Delimiter */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x03, /* Sub-function: Change ASCII Delimiter */
		'\t', 0x00, /* New delimiter: CR (0x0D) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_DIAGNOSTICS, res[0]);
	ASSERT_EQ(0x00, res[1]); /* Sub-function H */
	ASSERT_EQ(0x03, res[2]); /* Sub-function L */
	ASSERT_EQ('\t', res[3]); /* Echo data H */
	ASSERT_EQ(0x00, res[4]); /* Echo data L */

	/* Verify delimiter was changed */
	ASSERT_EQ('\t', inst.state.ascii_delimiter);
}

TEST(mbdiag_force_listen_only_mode_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Sub-function 0x04: Force Listen Only Mode */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x04, /* Sub-function: Force Listen Only */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	/* Listen only mode means no response */
	ASSERT_EQ(0u, res_size);

	/* Verify device is now in listen only mode */
	ASSERT_EQ(1, inst.state.is_listen_only);
}

TEST(mbdiag_force_listen_only_invalid_data_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x04, /* Sub-function: Force Listen Only */
		0x12, 0x34, /* Invalid data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);

	ASSERT_EQ(0, inst.state.is_listen_only);
}

static int s_reset_called = 0;
static void test_reset_diagnostics_callback(void) {
	s_reset_called = 1;
}
TEST(mbdiag_clear_counters_and_diagnostic_register_works)
{
	struct mbinst_s inst = {
		.serial = {
			.reset_diagnostics_cb = test_reset_diagnostics_callback
		}
	};
	mbinst_init(&inst);

	/* Set some counter values first */
	inst.state.bus_msg_counter = 10;
	inst.state.bus_comm_err_counter = 5;
	inst.state.exception_counter = 2;
	s_reset_called = 0;

	/* Sub-function 0x0A: Clear Counters and Diagnostic Register */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x0A, /* Sub-function: Clear Counters */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_DIAGNOSTICS, res[0]);
	ASSERT_EQ(0x00, res[1]); /* Sub-function H */
	ASSERT_EQ(0x0A, res[2]); /* Sub-function L */
	ASSERT_EQ(0x00, res[3]); /* Data H */
	ASSERT_EQ(0x00, res[4]); /* Data L */

	/* Verify counters are cleared */
	ASSERT_EQ(0, inst.state.bus_msg_counter);
	ASSERT_EQ(0, inst.state.bus_comm_err_counter);
	ASSERT_EQ(0, inst.state.exception_counter);
	ASSERT_EQ(1, s_reset_called); /* Callback should be invoked */
}

TEST(mbdiag_clear_counters_invalid_data_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x0A, /* Sub-function: Clear Counters */
		0x12, 0x34, /* Invalid data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbdiag_return_bus_message_count_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.bus_msg_counter = 0x1234;

	/* Sub-function 0x0B: Return Bus Message Count */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x0B, /* Sub-function: Bus Message Count */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_DIAGNOSTICS, res[0]);
	ASSERT_EQ(0x00, res[1]); /* Sub-function H */
	ASSERT_EQ(0x0B, res[2]); /* Sub-function L */
	ASSERT_EQ(0x12, res[3]); /* Counter H */
	ASSERT_EQ(0x34, res[4]); /* Counter L */
}

TEST(mbdiag_return_bus_comm_error_count_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.bus_comm_err_counter = 0x5678;

	/* Sub-function 0x0C: Return Bus Communication Error Count */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x0C, /* Sub-function: Bus Communication Error Count */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x56, res[3]); /* Counter H */
	ASSERT_EQ(0x78, res[4]); /* Counter L */
}

TEST(mbdiag_return_bus_exception_error_count_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.exception_counter = 0x9ABC;

	/* Sub-function 0x0D: Return Bus Exception Error Count */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x0D, /* Sub-function: Bus Exception Error Count */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x9A, res[3]); /* Counter H */
	ASSERT_EQ(0xBC, res[4]); /* Counter L */
}

TEST(mbdiag_return_server_message_count_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.msg_counter = 0xDEF0;

	/* Sub-function 0x0E: Return Server Message Count */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x0E, /* Sub-function: Server Message Count */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0xDE, res[3]); /* Counter H */
	ASSERT_EQ(0xF0+1, res[4]); /* Counter L */
}

TEST(mbdiag_return_server_no_response_count_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.no_resp_counter = 0x1357;

	/* Sub-function 0x0F: Return Server No Response Count */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x0F, /* Sub-function: Server No Response Count */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x13, res[3]); /* Counter H */
	ASSERT_EQ(0x57, res[4]); /* Counter L */
}

TEST(mbdiag_return_server_nak_count_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.nak_counter = 0x2468;

	/* Sub-function 0x10: Return Server NAK Count */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x10, /* Sub-function: Server NAK Count */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x24, res[3]); /* Counter H */
	ASSERT_EQ(0x68, res[4]); /* Counter L */
}

TEST(mbdiag_return_server_busy_count_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.busy_counter = 0x3691;

	/* Sub-function 0x11: Return Server Busy Count */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x11, /* Sub-function: Server Busy Count */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x36, res[3]); /* Counter H */
	ASSERT_EQ(0x91, res[4]); /* Counter L */
}

TEST(mbdiag_return_bus_character_overrun_count_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.bus_char_overrun_counter = 0x4815;

	/* Sub-function 0x12: Return Bus Character Overrun Count */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x12, /* Sub-function: Bus Character Overrun Count */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(0x48, res[3]); /* Counter H */
	ASSERT_EQ(0x15, res[4]); /* Counter L */
}

TEST(mbdiag_clear_overrun_counter_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set a known value */
	inst.state.bus_char_overrun_counter = 100;

	/* Sub-function 0x14: Clear Overrun Counter and Flag */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x14, /* Sub-function: Clear Overrun Counter */
		0x00, 0x00, /* Data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_DIAGNOSTICS, res[0]);
	ASSERT_EQ(0x00, res[1]); /* Sub-function H */
	ASSERT_EQ(0x14, res[2]); /* Sub-function L */
	ASSERT_EQ(0x00, res[3]); /* Data H */
	ASSERT_EQ(0x00, res[4]); /* Data L */

	/* Verify overrun counter is cleared */
	ASSERT_EQ(0, inst.state.bus_char_overrun_counter);
}

TEST(mbdiag_unknown_subfunction_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Unknown sub-function */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0xFF, /* Unknown sub-function */
		0x00, 0x00, /* Data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_FN, res[1]);
}

TEST(mbdiag_invalid_pdu_length_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Too short PDU (missing data) */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x01, /* Sub-function: Loopback */
		/* Missing data field */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbdiag_counter_validation_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Sub-function requiring 0x0000 data with non-zero data */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x0B, /* Sub-function: Bus Message Count */
		0x01, 0x00, /* Invalid data (must be 0x0000) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

/* Test function code 0x07 (MBFC_READ_EXCEPTION_STATUS) */

static uint8_t s_test_exception_status = 0x42;
static uint8_t test_read_exception_status_callback(void) {
	return s_test_exception_status;
}
TEST(mbdiag_read_exception_status_works)
{
	struct mbinst_s inst = {
		.serial = {
			.read_exception_status_cb = test_read_exception_status_callback
		}
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_EXCEPTION_STATUS,
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_EXCEPTION_STATUS, res[0]);
	ASSERT_EQ(0x42, res[1]); /* Exception status */
}

/* Test function code 0x0B (MBFC_COMM_EVENT_COUNTER) */

TEST(mbdiag_get_comm_event_counter_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set known values */
	inst.state.status = 0x1234;
	inst.state.comm_event_counter = 0x5678;

	uint8_t pdu_data[] = {
		MBFC_COMM_EVENT_COUNTER,
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(5u, res_size);
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_COMM_EVENT_COUNTER, res[0]);
	ASSERT_EQ(0x12, res[1]); /* Status H */
	ASSERT_EQ(0x34, res[2]); /* Status L */
	ASSERT_EQ(0x56, res[3]); /* Event counter H */
	ASSERT_EQ(0x78, res[4]); /* Event counter L */
}

TEST(mbdiag_get_comm_event_counter_invalid_length_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_COMM_EVENT_COUNTER,
		0x00, /* Extra data (should be none) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

/* Test function code 0x0C (MBFC_COMM_EVENT_LOG) */

TEST(mbdiag_get_comm_event_log_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Set known values */
	inst.state.status = 0x1234;
	inst.state.comm_event_counter = 0x5678;
	inst.state.event_log[0] = 0xAA;
	inst.state.event_log[1] = 0xBB;
	inst.state.event_log[2] = 0xCC;
	inst.state.event_log_count = 3;
	inst.state.event_log_write_pos = 3;

	uint8_t pdu_data[] = {MBFC_COMM_EVENT_LOG};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(11u, res_size); /* 1 fc + 1 byte count + 2 status + 2 event counter + 2 msg count + 3 events */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_COMM_EVENT_LOG, res[0]);
	ASSERT_EQ(9u, res[1]); /* Byte count */
	ASSERT_EQ(0x12, res[2]); /* Status H */
	ASSERT_EQ(0x34, res[3]); /* Status L */
	ASSERT_EQ(0x56, res[4]); /* Event counter H */
	ASSERT_EQ(0x78, res[5]); /* Event counter L */
	ASSERT_EQ(0x00, res[6]); /* Message count H */
	ASSERT_EQ(0x00, res[7]); /* Message count L */
	ASSERT_EQ(0xCC, res[8]); /* Event 0 */
	ASSERT_EQ(0xBB, res[9]); /* Event 1 */
	ASSERT_EQ(0xAA, res[10]); /* Event 2 */
}

TEST(mbdiag_get_comm_event_log_empty_works)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* No events in log */
	inst.state.status = 0x0000;
	inst.state.comm_event_counter = 0x0000;
	inst.state.event_log_count = 0;

	uint8_t pdu_data[] = {
		MBFC_COMM_EVENT_LOG,
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(8u, res_size); /* 1 fc + 1 byte count + 2 status + 2 event counter + 2 msg count */
	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_COMM_EVENT_LOG, res[0]);
	ASSERT_EQ(6u, res[1]); /* Byte count */
	ASSERT_EQ(0x00, res[2]); /* Status H */
	ASSERT_EQ(0x00, res[3]); /* Status L */
	ASSERT_EQ(0x00, res[4]); /* Event counter H */
	ASSERT_EQ(0x00, res[5]); /* Event counter L */
	ASSERT_EQ(0x00, res[6]); /* Message count H */
	ASSERT_EQ(0x00, res[7]); /* Message count L */
}

TEST(mbdiag_get_comm_event_log_invalid_length_fails)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_COMM_EVENT_LOG,
		0x00, /* Extra data (should be none) */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

/* Test edge cases and validation */

TEST(mbdiag_listen_only_mode_blocks_requests)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Put device in listen only mode */
	inst.state.is_listen_only = 1;

	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x00, /* Sub-function: Loopback */
		0x12, 0x34, /* Test data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	/* Should not respond when in listen only mode */
	ASSERT_EQ(0u, res_size);
}

TEST(mbdiag_listen_only_allows_restart_comms)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	/* Put device in listen only mode */
	inst.state.is_listen_only = 1;

	/* Restart communications should work even in listen only mode */
	uint8_t pdu_data[] = {
		MBFC_DIAGNOSTICS,
		0x00, 0x01, /* Sub-function: Restart Communications */
		0x00, 0x00, /* Data */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT_EQ(0u, res_size); /* No response */
	ASSERT_EQ(0, inst.state.is_listen_only); /* Device should exit listen only mode */
}

TEST_MAIN(
	mbdiag_loopback_works,
	mbdiag_restart_comms_works,
	mbdiag_restart_comms_in_listen_only_works,
	mbdiag_restart_comms_with_callback,
	mbdiag_restart_comms_invalid_data_fails,
	mbdiag_read_diagnostic_register_works,
	mbdiag_read_diagnostic_register_no_callback,
	mbdiag_read_diagnostic_register_invalid_data_fails,
	mbdiag_change_ascii_delimiter_works,
	mbdiag_force_listen_only_mode_works,
	mbdiag_force_listen_only_invalid_data_fails,
	mbdiag_clear_counters_and_diagnostic_register_works,
	mbdiag_clear_counters_invalid_data_fails,
	mbdiag_return_bus_message_count_works,
	mbdiag_return_bus_comm_error_count_works,
	mbdiag_return_bus_exception_error_count_works,
	mbdiag_return_server_message_count_works,
	mbdiag_return_server_no_response_count_works,
	mbdiag_return_server_nak_count_works,
	mbdiag_return_server_busy_count_works,
	mbdiag_return_bus_character_overrun_count_works,
	mbdiag_clear_overrun_counter_works,
	mbdiag_unknown_subfunction_fails,
	mbdiag_invalid_pdu_length_fails,
	mbdiag_counter_validation_works,
	mbdiag_read_exception_status_works,
	mbdiag_get_comm_event_counter_works,
	mbdiag_get_comm_event_counter_invalid_length_fails,
	mbdiag_get_comm_event_log_works,
	mbdiag_get_comm_event_log_empty_works,
	mbdiag_get_comm_event_log_invalid_length_fails,
	mbdiag_listen_only_mode_blocks_requests,
	mbdiag_listen_only_allows_restart_comms
);
