/**
 * @file mbadu_ascii.c
 * @brief Implementation of Modbus Ascii Application Data Unit handling
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

#include "mbadu_ascii.h"
#include "mbadu.h"
#include "mbpdu.h"
#include <ctype.h>

static int xtoi(char c)
{
	if (c>='0' && c<='9') return c - '0';
	if (c>='A' && c<='F') return c - 'A' + 10;
	if (c>='a' && c<='f') return c - 'a' + 10;
	return -1; /* Invalid hex char */
}

static void u8tox(uint8_t v, uint8_t *out)
{
	const char *hex_digits = "0123456789ABCDEF";
	out[0] = hex_digits[(v >> 4) & 0x0F];
	out[1] = hex_digits[v & 0x0F];
}

static uint8_t calc_lrc(const uint8_t *data, int len)
{
	int i, sum;

	/* Calculate sum of all bytes */
	sum = 0;
	for (i=0; i<len; ++i) {
		sum += data[i];
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
		res_size += 2;
	}

	u8tox(calc_lrc(bin_res, bin_res_len), res+res_size);
	res_size += 2;

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
	int lrc_err;

	/* Use response buffer to store temporary binary request */
	uint8_t *req_bin = res;
	uint8_t res_bin[1+MBPDU_SIZE_MAX];

	if (!inst || !req || !res) return 0;
	if (req_len<MBADU_ASCII_SIZE_MIN || req_len>MBADU_ASCII_SIZE_MAX) return 0;

	++inst->state.bus_msg_counter;

	/* Ensure correct start and end chars */
	if (req[0] != MBADU_ASCII_START_CHAR) return 0;
	if (req[req_len-2]!='\r' || req[req_len-1]!=inst->state.ascii_delimiter) return 0;

	/* Ensure length of request without start char is divisible by two */
	if ((req_len-1)%2 != 0) return 0;

	/* Ensure entire request (excluding start and end chars) are hex */
	for (i=1; i<req_len-2; ++i) {
		if (!isxdigit(req[i])) return 0;
	}

	/* Convert ascii request to binary */
	req_bin_len = 0;
	for (i=1; i<req_len-2; i+=2) { /* Excluding start char end crlf */
		req_bin[req_bin_len++] = (uint8_t)(xtoi(req[i])*16 + xtoi(req[i+1]));
	}

	recv_slave_addr = req_bin[0];
	if (recv_slave_addr != inst->serial.slave_addr
			&& recv_slave_addr != MBADU_ADDR_BROADCAST
			&& (!inst->serial.enable_def_resp || recv_slave_addr != MBADU_ADDR_DEFAULT_RESP)) {
		return 0;
	}

	recv_lrc = req_bin[req_bin_len-1];
	lrc_err = recv_lrc != calc_lrc(req_bin, req_bin_len-1);

	/* Create receive communication log entry */
	if (lrc_err
			|| recv_slave_addr==MBADU_ADDR_BROADCAST
			|| inst->state.is_listen_only) {
		mb_add_comm_event(
			inst,
			MB_COMM_EVENT_IS_RECV
			| (lrc_err ? MB_COMM_EVENT_RECV_COMM_ERR : 0)
			| (inst->state.is_listen_only ? MB_COMM_EVENT_RECV_LISTEN_MODE : 0)
			| (recv_slave_addr==MBADU_ADDR_BROADCAST ? MB_COMM_EVENT_RECV_BROADCAST : 0));
	}

	if (lrc_err) {
		++inst->state.bus_comm_err_counter;
		return 0;
	}

	res_bin[0] = req_bin[0];
	res_pdu_len = mbpdu_handle_req(
		inst,
		req_bin+1, /* Skip slave address */
		req_bin_len-2, /* Do not include lrc */
		res_bin+1);

	/* Requests sent to the broadcast address shall never get a response */
	if (res_pdu_len==0u || recv_slave_addr==MBADU_ADDR_BROADCAST) {
		++inst->state.no_resp_counter;
		return 0;
	}

	return prep_res(inst, res_bin, 1+res_pdu_len, res);
}
