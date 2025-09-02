/**
 * @file mbfn_digs.c
 * @brief Implementation of Modbus diagnostic function handlers
 * @author Jonas Almås
 */

/*
 * Copyright (c) 2025 Siemens Energy AS
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OR CONDITIONS OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OR CONDITIONS
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE) OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authorized representative: Edgar Vorland, SE TI EAD MF&P SUS OMS, Group Manager Electronics
 */

#include "mbfn_digs.h"
#include "utils/endian.h"
#include <string.h>

static void reset_comm_counters(struct mbinst_s *inst)
{
	inst->state.comm_event_counter = 0u;

	inst->state.bus_msg_counter = 0u;
	inst->state.bus_comm_err_counter = 0u;
	inst->state.exception_counter = 0u;
	inst->state.msg_counter = 0u;
	inst->state.no_resp_counter = 0u;
	inst->state.nak_counter = 0u;
	inst->state.busy_counter = 0u;
	inst->state.bus_char_overrun_counter = 0u;
}

/**
 * @brief 0x00 Return Query Data
 */
static enum mbstatus_e loopback(
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	memcpy(res->p, req, req_len);
	res->size = req_len;
	return MB_OK;
}

/**
 * @brief 0x01 Restart Communications Option
 */
static enum mbstatus_e restart_comms_opt(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	uint16_t val;

	if (req_len != 5) return MB_ILLEGAL_DATA_VAL;

	val = betou16(req+3);
	if (val!=0x0000 && val!=0xFF00) return MB_ILLEGAL_DATA_VAL;

	if (inst->serial.request_restart) {
		inst->serial.request_restart();
	}
	inst->state.is_listen_only = 0;
	reset_comm_counters(inst);

	if (val==0xFF00) { /* Clear event log ring buffer */
		inst->state.event_log_write_pos = 0;
		inst->state.event_log_count = 0;
	} else {
		mb_add_comm_event(inst, MB_COMM_EVENT_COMM_RESTART);
	}

	u16tobe(val, res->p+3);
	res->size += 2;

	return MB_OK;
}

/**
 * @brief 0x02 Return Diagnostic Register
 */
static enum mbstatus_e read_diagnostic_reg(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	if (req_len != 5) return MB_ILLEGAL_DATA_VAL;
	if (betou16(req+3) != 0) return MB_ILLEGAL_DATA_VAL;

	if (inst->serial.read_diagnostics_cb) {
		u16tobe(inst->serial.read_diagnostics_cb(), res->p+3);
	} else {
		u16tobe(0, res->p+3);
	}
	res->size += 2;

	return MB_OK;
}

/**
 * @brief 0x03 Change ASCII Input Delimiter
 */
static enum mbstatus_e change_ascii_delimiter(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	if (req_len != 5) return MB_ILLEGAL_DATA_VAL;
	if (req[3] > 127) return MB_ILLEGAL_DATA_VAL;
	if (req[4] != 0) return MB_ILLEGAL_DATA_VAL;

	inst->state.ascii_delimiter = req[3];

	res->p[3] = req[3];
	res->p[4] = 0u;
	res->size += 2;

	return MB_OK;
}

/**
 * @brief 0x04 Force Listen Only Mode
 */
static enum mbstatus_e force_listen_only(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len)
{
	if (req_len != 5) return MB_ILLEGAL_DATA_VAL;
	if (betou16(req+3) != 0) return MB_ILLEGAL_DATA_VAL;

	inst->state.is_listen_only = 1;
	mb_add_comm_event(inst, MB_COMM_EVENT_ENTERED_LISTEN_ONLY);

	return MB_OK;
}

/**
 * @brief 0x0A Clear Counters and Diagnostic Register
 */
static enum mbstatus_e clear_counts_n_diag_reg(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	if (req_len != 5) return MB_ILLEGAL_DATA_VAL;
	if (betou16(req+3) != 0) return MB_ILLEGAL_DATA_VAL;

	reset_comm_counters(inst);
	if (inst->serial.reset_diagnostics_cb) {
		inst->serial.reset_diagnostics_cb();
	}

	res->p[3] = 0u;
	res->p[4] = 0u;
	res->size += 2;

	return MB_OK;
}

static enum mbstatus_e read_counter(
	uint16_t counter_value,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	if (req_len != 5) return MB_ILLEGAL_DATA_VAL;
	if (betou16(req+3) != 0) return MB_ILLEGAL_DATA_VAL;

	u16tobe(counter_value, res->p+3);
	res->size += 2;

	return MB_OK;
}

/**
 * @brief 0x14 Clear Overrun Counter and Flag
 */
static enum mbstatus_e clr_overrun(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	if (req_len != 5) return MB_ILLEGAL_DATA_VAL;
	if (betou16(req+3) != 0) return MB_ILLEGAL_DATA_VAL;

	inst->state.bus_char_overrun_counter = 0u;

	res->p[3] = 0u;
	res->p[4] = 0u;
	res->size += 2;

	return MB_OK;
}

extern enum mbstatus_e mbfn_digs(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	if (!inst || !req || !res) return MB_DEV_FAIL;

	if (req_len < 3) return MB_ILLEGAL_DATA_VAL;

	/* Always echo function code and sub-function code */
	res->p[0] = req[0]; /* Fc */
	res->p[1] = req[1]; /* Sub-fc H */
	res->p[2] = req[2]; /* Sub-fc L */
	res->size = 3;

	switch (betou16(req+1)) {
	case MBFC_DIGS_LOOPBACK: return loopback(req, req_len, res);
	case MBFC_DIGS_RESTART_COMMS_OPT: return restart_comms_opt(inst, req, req_len, res);
	case MBFC_DIGS_REG: return read_diagnostic_reg(inst, req, req_len, res);
	case MBFC_DIGS_ASCII_DELIM: return change_ascii_delimiter(inst, req, req_len, res);
	case MBFC_DIGS_FORCE_LISTEN: return force_listen_only(inst, req, req_len);
	case MBFC_DIGS_CLR_CNTS_N_DIAG_REG: return clear_counts_n_diag_reg(inst, req, req_len, res);
	case MBFC_DIGS_BUS_MSG_COUNT: return read_counter(inst->state.bus_msg_counter, req, req_len, res);
	case MBFC_DIGS_BUS_COMM_ERR_COUNT: return read_counter(inst->state.bus_comm_err_counter, req, req_len, res);
	case MBFC_DIGS_BUS_EXCEPTION_COUNT: return read_counter(inst->state.exception_counter, req, req_len, res);
	case MBFC_DIGS_MSG_COUNT: return read_counter(inst->state.msg_counter, req, req_len, res);
	case MBFC_DIGS_NO_RESP_MSG_COUNT: return read_counter(inst->state.no_resp_counter, req, req_len, res);
	case MBFC_DIGS_NAK_COUNT: return read_counter(inst->state.nak_counter, req, req_len, res);
	case MBFC_DIGS_BUSY_COUNT: return read_counter(inst->state.busy_counter, req, req_len, res);
	case MBFC_DIGS_BUS_OVERRUN_COUNT: return read_counter(inst->state.bus_char_overrun_counter, req, req_len, res);
	case MBFC_DIGS_CLR_OVERRUN: return clr_overrun(inst, req, req_len, res);
	default: return MB_ILLEGAL_FN;
	}
}

extern enum mbstatus_e mbfn_comm_event_counter(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	if (!inst || !req || !res) return MB_DEV_FAIL;
	if (req_len != 1) return MB_ILLEGAL_DATA_VAL;

	u16tobe(inst->state.status, res->p+1);
	u16tobe(inst->state.comm_event_counter, res->p+3);
	res->size = 5;

	return MB_OK;
}

extern enum mbstatus_e mbfn_comm_event_log(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	int i, ix;

	if (!inst || !req || !res) return MB_DEV_FAIL;
	if (req_len != 1) return MB_ILLEGAL_DATA_VAL;

	res->p[1] = 6 + inst->state.event_log_count; /* Byte count */
	u16tobe(inst->state.status, res->p+2);
	u16tobe(inst->state.comm_event_counter, res->p+4);
	u16tobe(inst->state.bus_msg_counter, res->p+6);
	res->size = 8;

	/* Read comm log starting with the newest message */
	for (i=0; i<inst->state.event_log_count; ++i) {
		ix = (inst->state.event_log_write_pos + MB_COMM_EVENT_LOG_LEN - 1 - i) % MB_COMM_EVENT_LOG_LEN;
		res->p[res->size++] = inst->state.event_log[ix];
	}

	return MB_OK;
}
