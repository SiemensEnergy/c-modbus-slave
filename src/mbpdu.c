/**
 * @file mbpdu.c
 * @brief Implementation of Modbus Protocol Data Unit (PDU) handling
 * @author Jonas Almås
 *
 * MISRA Deviations:
 * - Rule 15.5: A function should have a single point of exit at the end
 *   Rationale: Multiple returns improve readability and reduce nesting
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

#include "mbpdu.h"
#include "endian.h"
#include "mbdef.h"
#include "mbfn_coils.h"
#include "mbfn_digs.h"
#include "mbfn_regs.h"
#include "mbfn_serial.h"
#include <stddef.h>
#include <stdint.h>

static enum mbstatus_e handle(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	switch (req[0]) {
	case MBFC_READ_COILS:
		if (inst->coils!=NULL) {
			return mbfn_read_coils(inst, inst->coils, inst->n_coils, req, req_len, res);
		}
		break;
	case MBFC_READ_DISC_INPUTS:
		if (inst->disc_inputs!=NULL) {
			return mbfn_read_coils(inst, inst->disc_inputs, inst->n_disc_inputs, req, req_len, res);
		}
		break;
	case MBFC_READ_HOLDING_REGS:
		if (inst->hold_regs!=NULL) {
			return mbfn_read_regs(inst, inst->hold_regs, inst->n_hold_regs, req, req_len, res);
		}
		break;
	case MBFC_READ_INPUT_REGS:
		if (inst->input_regs!=NULL) {
			return mbfn_read_regs(inst, inst->input_regs, inst->n_input_regs, req, req_len, res);
		}
		break;
	case MBFC_WRITE_SINGLE_COIL:
		if (inst->coils!=NULL) {
			return mbfn_write_coil(inst, inst->coils, inst->n_coils, req, req_len, res);
		}
		break;
	case MBFC_WRITE_SINGLE_REG:
		if (inst->hold_regs!=NULL) {
			return mbfn_write_reg(inst, inst->hold_regs, inst->n_hold_regs, req, req_len, res);
		}
		break;
	case MBFC_READ_EXCEPTION_STATUS:
		if (inst->serial.read_exception_status_cb!=NULL) {
			return mbfn_read_exception_status(inst, req, req_len, res);
		}
		break;
	case MBFC_DIAGNOSTICS: return mbfn_digs(inst, req, req_len, res);
	case MBFC_COMM_EVENT_COUNTER: return mbfn_comm_event_counter(inst, req, req_len, res);
	case MBFC_COMM_EVENT_LOG: return mbfn_comm_event_log(inst, req, req_len, res);
	case MBFC_WRITE_MULTIPLE_COILS:
		if (inst->coils!=NULL) {
			return mbfn_write_coils(inst, inst->coils, inst->n_coils, req, req_len, res);
		}
		break;
	case MBFC_WRITE_MULTIPLE_REGS:
		if (inst->hold_regs!=NULL) {
			return mbfn_write_regs(inst, inst->hold_regs, inst->n_hold_regs, req, req_len, res);
		}
		break;
	case MBFC_REPORT_SLAVE_ID: break; /* Should be implemented through mbinst_s::handle_fn_cb */
	case MBFC_READ_FILE_RECORD: break; /* Not implemented */
	case MBFC_WRITE_FILE_RECORD: break; /* Not implemented */
	case MBFC_MASK_WRITE_REG: break; /* Not implemented */
	case MBFC_READ_WRITE_REGS:
		if (inst->hold_regs!=NULL) {
			return mbfn_read_write_regs(inst, inst->hold_regs, inst->n_hold_regs, req, req_len, res);
		}
		break;
	case MBFC_READ_FIFO_QUEUE: break; /* Not implemented */
	default: break;
	}

	if (inst->handle_fn_cb!=NULL) {
		return inst->handle_fn_cb(inst, req, req_len, res);
	} else {
		return MB_ILLEGAL_FN;
	}
}

extern size_t mbpdu_handle_req(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	uint8_t *res)
{
	int was_listen_only;
	uint8_t send_event;
	enum mbstatus_e status;
	struct mbpdu_buf_s res_pdu;

	if ((inst==NULL) || (req==NULL) || (res==NULL)) return 0u;
	if (req_len<1u) return 0u;

	send_event = MB_COMM_EVENT_IS_SEND;

	/* If we are in listen mode we don't handle any requests,
	   other than reset communication. */
	if (inst->state.is_listen_only
			&& ((req_len<3u)
				|| (req[0]!=MBFC_DIAGNOSTICS)
				|| (betou16(req+1u)!=MBFC_DIGS_RESTART_COMMS_OPT))) {
		send_event |= MB_COMM_EVENT_SEND_LISTEN_ONLY;
		mb_add_comm_event(inst, send_event);
		return 0u;
	}

	/* Increment count of messages addressed to this device.
	   Should not get incremented when in listen only mode. */
	++inst->state.msg_counter;

	was_listen_only = inst->state.is_listen_only;

	/* Copy function code from request to response */
	res[0] = req[0];

	res_pdu.p = res;
	res_pdu.size = 1u;

	status = handle(inst, req, req_len, &res_pdu);

	if (status!=MB_OK) {
		/* Prepare exception response */
		res[0] |= MB_ERR_FLG;
		res[1] = status;
		res_pdu.size = 2u;

		/* Set send communication event flags */
		if ((status==MB_ILLEGAL_FN)
				|| (status==MB_ILLEGAL_DATA_ADDR)
				|| (status==MB_ILLEGAL_DATA_VAL)) {
			send_event |= MB_COMM_EVENT_SEND_READ_EX;
		}
		if (status==MB_DEV_FAIL) {send_event |= MB_COMM_EVENT_SEND_ABORT_EX;}
		if ((status==MB_ACK) || (status==MB_BUSY)) {send_event |= MB_COMM_EVENT_SEND_BUSY_EX;}
		if (status==MB_NEG_ACK) {send_event |= MB_COMM_EVENT_SEND_NAK_EX;}
	}

	/* Listen only mode changes "takes effect" after the response is sent,
	   therefore we report the state as before handling the request. */
	if (was_listen_only != 0) {send_event |= MB_COMM_EVENT_SEND_LISTEN_ONLY;}
	mb_add_comm_event(inst, send_event);

	/* Increment diagnostic counters */
	if ((status==MB_OK)
			&& (req[0]!=MBFC_DIAGNOSTICS)
			&& (req[0]!=MBFC_COMM_EVENT_COUNTER)
			&& (req[0]!=MBFC_COMM_EVENT_LOG)) {
		++inst->state.comm_event_counter;
	}
	if (status!=MB_OK) {++inst->state.exception_counter;}
	if (status==MB_NEG_ACK) {++inst->state.nak_counter;}
	if (status==MB_BUSY) {++inst->state.busy_counter;}

	/* If the device is in listen only more, or was prior to this request;
	   we don't want to send a response. */
	return (inst->state.is_listen_only || was_listen_only)
		? 0u
		: res_pdu.size;
}
