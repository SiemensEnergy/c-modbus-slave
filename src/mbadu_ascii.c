/**
 * @file mbadu_ascii.c
 * @brief Implementation of Modbus Ascii Application Data Unit handling
 * @author Jonas Almås
 *
 * MISRA Deviations:
 * - Rule 10.2: Expressions of essentially character type shall not be used inappropriately in addition and subtraction operations
 * - Rule 10.4: Both operands of an operator in which the usual arithmetic conversions are performed shall have the same essential type category
 * - Rule 10.8: The value of a composite expression shall not be cast to a different essential type category or a wider essential type
 * - Rule 12.3: The comma operator should not be used
 * - Rule 13.3: A full expression containing an increment (++) or decrement (--) operator should have no other potential side effects other than that caused by the increment or decrement
 * - Rule 14.4: The controlling expression of an if statement and the controlling expression of an iteration-statement shall have essentially Boolean type
 * - Rule 15.5: A function should have a single point of exit at the end
 * - Rule 18.4: The +, -, += and -= operators should not be applied to an expression of pointer type
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

#include "mbadu_ascii.h"
#include "mbadu.h"
#include "mbpdu.h"
#include <ctype.h>

static int xtoi(int c)
{
	if ((c>='0') && (c<='9')) return c - '0';
	if ((c>='A') && (c<='F')) return c - 'A' + 10;
	if ((c>='a') && (c<='f')) return c - 'a' + 10;
	return -1; /* Invalid hex char */
}

static void u8tox(uint8_t v, uint8_t *out)
{
	const char *hex_digits = "0123456789ABCDEF";
	out[0] = hex_digits[(v >> 4) & 0x0Fu];
	out[1] = hex_digits[v & 0x0Fu];
}

static uint8_t calc_lrc(const uint8_t *data, int len)
{
	int i, sum;

	/* Calculate sum of all bytes */
	sum = 0;
	for (i=0; i<len; ++i) {
		sum += (int)data[i];
	}

	/* Get two's complement and trim to 8 bits */
	return (uint8_t)(-sum);
}

static size_t prep_res(
	const struct mbinst_s *inst,
	const uint8_t *bin_res,
	size_t bin_res_len,
	uint8_t *res)
{
	size_t i, res_size;

	res_size = 0;
	res[res_size++] = MBADU_ASCII_START_CHAR;

	/* Convert binary PDU to ascii  */
	for (i=0; i<bin_res_len; ++i) {
		u8tox(bin_res[i], res+res_size);
		res_size += 2u;
	}

	u8tox(calc_lrc(bin_res, bin_res_len), res+res_size);
	res_size += 2u;

	res[res_size++] = '\r';
	res[res_size++] = inst->state.ascii_delimiter;

	return res_size;
}

extern size_t mbadu_ascii_handle_req(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	uint8_t *res)
{
	size_t i, req_bin_len, res_pdu_len;
	uint8_t recv_slave_addr, recv_lrc;
	uint8_t recv_event;

	/* Use response buffer to store temporary binary request */
	uint8_t *req_bin = res;
	uint8_t res_bin[1u+MBPDU_SIZE_MAX];

	if (!inst || !req || !res) return 0u;
	if ((req_len<MBADU_ASCII_SIZE_MIN) || (req_len>MBADU_ASCII_SIZE_MAX)) return 0u;

	++inst->state.bus_msg_counter;

	recv_event = 0u;
	if (inst->state.is_listen_only) {recv_event |= MB_COMM_EVENT_RECV_LISTEN_MODE;}

	/* Ensure correct start and end chars, and length without start char is divisible by two (ascii hex) */
	if ((req[0] != MBADU_ASCII_START_CHAR)
			|| (req[req_len-2u] != '\r')
			|| (req[req_len-1u] != inst->state.ascii_delimiter)
			|| ((req_len-1u)%2u != 0u)) {
		if (recv_event) mb_add_comm_event(inst, MB_COMM_EVENT_IS_RECV | recv_event);
		return 0u;
	}

	/* Ensure entire request (excluding start and end chars) are hex */
	for (i=1u; i<(req_len-2u); ++i) {
		if (!isxdigit(req[i])) {
			if (recv_event) mb_add_comm_event(inst, MB_COMM_EVENT_IS_RECV | recv_event);
			return 0u;
		}
	}

	/* Convert ascii request to binary */
	req_bin_len = 0u;
	for (i=1u; i<(req_len-2u); i+=2u) { /* Excluding start char end crlf */
		req_bin[req_bin_len++] = (uint8_t)(xtoi(req[i])*16 + xtoi(req[i+1u]));
	}

	/* Check LRC before slave address to monitor the overall health of the
	   bus, not just this device */
	recv_lrc = req_bin[req_bin_len-1u];
	if (recv_lrc != calc_lrc(req_bin, req_bin_len-1u)) {
		++inst->state.bus_comm_err_counter;
		recv_event |= MB_COMM_EVENT_RECV_COMM_ERR;
		mb_add_comm_event(inst, MB_COMM_EVENT_IS_RECV | recv_event);
		return 0u;
	}

	/* Check if this request is addressed to this device */
	recv_slave_addr = req_bin[0];
	if ((recv_slave_addr != inst->serial.slave_addr)
			&& (recv_slave_addr != MBADU_ADDR_BROADCAST)
			&& (!inst->serial.enable_def_resp || (recv_slave_addr != MBADU_ADDR_DEFAULT_RESP))) {
		if (recv_event) mb_add_comm_event(inst, MB_COMM_EVENT_IS_RECV | recv_event);
		return 0u;
	}

	if (recv_slave_addr==MBADU_ADDR_BROADCAST) {recv_event |= MB_COMM_EVENT_RECV_BROADCAST;}
	if (recv_event) {mb_add_comm_event(inst, MB_COMM_EVENT_IS_RECV | recv_event);}

	res_bin[0] = req_bin[0];
	res_pdu_len = mbpdu_handle_req(
		inst,
		req_bin+1u, /* Skip slave address */
		req_bin_len-2u, /* - Slave address and lrc */
		res_bin+1u);

	/* Requests sent to the broadcast address shall never get a response */
	if ((res_pdu_len==0u) || (recv_slave_addr==MBADU_ADDR_BROADCAST)) {
		++inst->state.no_resp_counter;
		return 0u;
	}

	return prep_res(inst, res_bin, 1u+res_pdu_len, res);
}
