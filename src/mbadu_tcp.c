/**
 * @file mbadu_tcp.c
 * @brief Implementation of Modbus TCP/IP Application Data Unit handling
 * @author Jonas Almås
 *
 * MISRA Deviations:
 * - Rule 12.3: The comma operator should not be used
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

#include "mbadu_tcp.h"
#include "endian.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern size_t mbadu_tcp_handle_req(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	uint8_t *res)
{
	size_t pdu_size;
	uint16_t transaction_id, protocol_id, length;
	uint8_t unit_id;

	if (!inst || !req || !res) return 0u;

	/* We must at least have a complate header + function code */
	if (req_len < (MBAP_SIZE+1u)) {
		return 0u;
	}

	transaction_id = betou16(req + MBAP_POS_TRANS_ID);
	protocol_id = betou16(req + MBAP_POS_PROT_ID);
	length = betou16(req + MBAP_POS_LEN);
	unit_id = req[MBAP_POS_UNIT_ID];

	if (protocol_id != MBADU_TCP_PROT_ID) {
		return 0u;
	}

	pdu_size = mbpdu_handle_req(
		inst,
		req + MBAP_SIZE,
		length-1u,
		res + MBAP_SIZE);

	if (pdu_size==0u) {
		return 0u;
	}

	/* Build response MBAP */
	u16tobe(transaction_id, res + MBAP_POS_TRANS_ID);
	u16tobe(protocol_id, res + MBAP_POS_PROT_ID);
	u16tobe(1u+pdu_size, res + MBAP_POS_LEN);
	res[MBAP_POS_UNIT_ID] = unit_id;

	return MBAP_SIZE + pdu_size;
}
