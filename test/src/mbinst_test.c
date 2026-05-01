#include "test_lib.h"
#include <mbinst.h>
#include <stdint.h>

TEST(mbinst_init_clears_is_listen_only)
{
	struct mbinst_s inst = {0};
	inst.state.is_listen_only = 1;
	mbinst_init(&inst);
	ASSERT_EQ(0, inst.state.is_listen_only);
}

TEST(mbinst_init_clears_status)
{
	struct mbinst_s inst = {0};
	inst.state.status = 0xFFFFu;
	mbinst_init(&inst);
	ASSERT_EQ(0u, inst.state.status);
}

TEST(mbinst_init_clears_comm_event_counter)
{
	struct mbinst_s inst = {0};
	inst.state.comm_event_counter = 1000u;
	mbinst_init(&inst);
	ASSERT_EQ(0u, inst.state.comm_event_counter);
}

TEST(mbinst_init_clears_event_log_state)
{
	struct mbinst_s inst = {0};
	inst.state.event_log_write_pos = 10;
	inst.state.event_log_count = 5;
	mbinst_init(&inst);
	ASSERT_EQ(0, inst.state.event_log_write_pos);
	ASSERT_EQ(0, inst.state.event_log_count);
}

TEST(mbinst_init_clears_bus_counters)
{
	struct mbinst_s inst = {0};
	inst.state.bus_msg_counter = 0xFFFFu;
	inst.state.bus_comm_err_counter = 0xFFFFu;
	inst.state.exception_counter = 0xFFFFu;
	inst.state.msg_counter = 0xFFFFu;
	inst.state.no_resp_counter = 0xFFFFu;
	inst.state.nak_counter = 0xFFFFu;
	inst.state.busy_counter = 0xFFFFu;
	inst.state.bus_char_overrun_counter = 0xFFFFu;
	mbinst_init(&inst);
	ASSERT_EQ(0u, inst.state.bus_msg_counter);
	ASSERT_EQ(0u, inst.state.bus_comm_err_counter);
	ASSERT_EQ(0u, inst.state.exception_counter);
	ASSERT_EQ(0u, inst.state.msg_counter);
	ASSERT_EQ(0u, inst.state.no_resp_counter);
	ASSERT_EQ(0u, inst.state.nak_counter);
	ASSERT_EQ(0u, inst.state.busy_counter);
	ASSERT_EQ(0u, inst.state.bus_char_overrun_counter);
}

TEST(mbinst_init_sets_ascii_delimiter_to_newline)
{
	struct mbinst_s inst = {0};
	inst.state.ascii_delimiter = 0xFFu;
	mbinst_init(&inst);
	ASSERT_EQ('\n', (char)inst.state.ascii_delimiter);
}

TEST(mb_add_comm_event_stores_event_at_pos_zero)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	mb_add_comm_event(&inst, 0xABu);
	ASSERT_EQ(0xABu, inst.state.event_log[0]);
}

TEST(mb_add_comm_event_increments_count)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	ASSERT_EQ(0, inst.state.event_log_count);
	mb_add_comm_event(&inst, 0x01u);
	ASSERT_EQ(1, inst.state.event_log_count);
	mb_add_comm_event(&inst, 0x02u);
	ASSERT_EQ(2, inst.state.event_log_count);
}

TEST(mb_add_comm_event_advances_write_pos)
{
	struct mbinst_s inst = {0};
	mbinst_init(&inst);
	mb_add_comm_event(&inst, 0x01u);
	ASSERT_EQ(1, inst.state.event_log_write_pos);
	mb_add_comm_event(&inst, 0x02u);
	ASSERT_EQ(2, inst.state.event_log_write_pos);
	ASSERT_EQ(0x01u, inst.state.event_log[0]);
	ASSERT_EQ(0x02u, inst.state.event_log[1]);
}

TEST(mb_add_comm_event_wraps_write_pos_at_log_len)
{
	struct mbinst_s inst = {0};
	int i;
	mbinst_init(&inst);
	for (i = 0; i < MB_COMM_EVENT_LOG_LEN; ++i) {
		mb_add_comm_event(&inst, (uint8_t)i);
	}
	ASSERT_EQ(0, inst.state.event_log_write_pos);
}

TEST(mb_add_comm_event_count_caps_at_log_len)
{
	struct mbinst_s inst = {0};
	int i;
	mbinst_init(&inst);
	for (i = 0; i < MB_COMM_EVENT_LOG_LEN + 10; ++i) {
		mb_add_comm_event(&inst, (uint8_t)i);
	}
	ASSERT_EQ(MB_COMM_EVENT_LOG_LEN, inst.state.event_log_count);
}

TEST(mb_add_comm_event_overwrites_oldest_when_full)
{
	struct mbinst_s inst = {0};
	int i;
	mbinst_init(&inst);
	/* Fill ring buffer with sequential values 0..63 */
	for (i = 0; i < MB_COMM_EVENT_LOG_LEN; ++i) {
		mb_add_comm_event(&inst, (uint8_t)i);
	}
	/* Next write wraps to index 0, overwriting the oldest entry */
	mb_add_comm_event(&inst, 0xFFu);
	ASSERT_EQ(0xFFu, inst.state.event_log[0]);
	ASSERT_EQ(MB_COMM_EVENT_LOG_LEN, inst.state.event_log_count);
}

TEST_MAIN(
	mbinst_init_clears_is_listen_only,
	mbinst_init_clears_status,
	mbinst_init_clears_comm_event_counter,
	mbinst_init_clears_event_log_state,
	mbinst_init_clears_bus_counters,
	mbinst_init_sets_ascii_delimiter_to_newline,
	mb_add_comm_event_stores_event_at_pos_zero,
	mb_add_comm_event_increments_count,
	mb_add_comm_event_advances_write_pos,
	mb_add_comm_event_wraps_write_pos_at_log_len,
	mb_add_comm_event_count_caps_at_log_len,
	mb_add_comm_event_overwrites_oldest_when_full
);
