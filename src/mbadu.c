/**
 * @file mbadu.c
 * @brief Implementation of Modbus Serial Application Data Unit handling
 * @author Jonas Almås
 *
 * MISRA Deviations:
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

#include "mbadu.h"
#include "endian.h"
#include "mbpdu.h"
#include "mbcrc.h"

/**
 * @brief Add Slave addres and CRC to response ADU
 */
static size_t prep_res(uint8_t slave_addr, uint8_t *res, size_t pdu_size)
{
	size_t res_size;
	uint16_t crc;

	res[0] = slave_addr;
	res_size = 1u + pdu_size;

	crc = mbcrc16(res, res_size);
	u16tole(crc, res+res_size);
	res_size += 2u;

	return res_size;
}

extern size_t mbadu_handle_req(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	uint8_t *res)
{
	uint8_t recv_event;
	uint8_t recv_slave_addr;
	uint16_t recv_crc;
	size_t pdu_size;

	if (!inst || !req || !res) return 0u;
	if ((req_len<MBADU_SIZE_MIN) || (req_len>MBADU_SIZE_MAX)) return 0u;

	++inst->state.bus_msg_counter;

	recv_event = 0;
	if (inst->state.is_listen_only) {recv_event |= MB_COMM_EVENT_RECV_LISTEN_MODE;}

	/* Check CRC before slave address to monitor the overall health of the
	   bus, not just this device */
	recv_crc = letou16(req + req_len - 2u); /* CRC is in the last two bytes, little endian */
	if (recv_crc != mbcrc16(req, req_len - 2u)) {
		++inst->state.bus_comm_err_counter;
		recv_event |= MB_COMM_EVENT_RECV_COMM_ERR;
		mb_add_comm_event(inst, MB_COMM_EVENT_IS_RECV | recv_event);
		return 0u;
	}

	/* Check if this request is addressed to this device */
	recv_slave_addr = req[0];
	if ((recv_slave_addr != inst->serial.slave_addr)
			&& (recv_slave_addr != MBADU_ADDR_BROADCAST)
			&& (!inst->serial.enable_def_resp || (recv_slave_addr != MBADU_ADDR_DEFAULT_RESP))) {
		if (recv_event) {mb_add_comm_event(inst, MB_COMM_EVENT_IS_RECV | recv_event);}
		return 0u;
	}

	if (recv_slave_addr==MBADU_ADDR_BROADCAST) {recv_event |= MB_COMM_EVENT_RECV_BROADCAST;}
	if (recv_event) {mb_add_comm_event(inst, MB_COMM_EVENT_IS_RECV | recv_event);}

	pdu_size = mbpdu_handle_req(
		inst,
		req+1u, /* Skip slave address */
		req_len-3u, /* - Slave address and crc */
		res+1u);

	/* Requests sent to the broadcast address shall never get a response */
	if ((pdu_size==0u) || (recv_slave_addr==MBADU_ADDR_BROADCAST)) {
		++inst->state.no_resp_counter;
		return 0u;
	}

	return prep_res(recv_slave_addr, res, pdu_size);
}
