/**
 * @file mbadu_tcp.h
 * @brief Modbus Application Data Unit (TCP/IP) - MBAP header wrapper around PDU
 * @author Jonas Almås
 *
 * @details This module provides TCP/IP specific Modbus Application Data Unit handling.
 * It wraps the Protocol Data Unit (PDU) with the Modbus Application Protocol
 * (MBAP) header for use in Modbus TCP/IP communications.
 *
 * @see mbpdu.h for the underlying PDU implementation
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

#ifndef MBADU_TCP_H_INCLUDED
#define MBADU_TCP_H_INCLUDED

#include "mbpdu.h"
#include <stddef.h>
#include <stdint.h>

/**
 * Modbus Application Protocol (MBAP) Header
 * - Transaction id (2 bytes BE)
 * - Protocol id (2 bytes BE)
 * - Length (2 bytes BE) (The rest of the buffer; including unit id)
 * - Unit id (1 byte) (Same as Modbus serial slave addres)
 */
enum {
	MBAP_POS_TRANS_ID = 0u,
	MBAP_POS_PROT_ID = 2u,
	MBAP_POS_LEN = 4u,
	MBAP_POS_UNIT_ID = 6u,

	MBAP_SIZE = 7u,
};

enum {MBADU_TCP_PROT_ID=0u};

enum {
	/*
	 * - 7 MBAP
	 * - 253 PDU
	 *   - 1 Function code
	 *   - 252 PDU data
	 */
	MBADU_TCP_SIZE_MAX = MBAP_SIZE + MBPDU_SIZE_MAX,
};

enum {MBTCP_PORT=502u};

/**
 * @brief Handle Modbus TCP/IP ADU request
 *
 * Processes a complete Modbus Application Data Unit for TCP/IP communication.
 * Validates the MBAP (Modbus Application Protocol) header, extracts the PDU,
 * processes the request, and generates a properly formatted response with MBAP header.
 *
 * @param inst Pointer to the Modbus instance containing coil/register maps and configuration
 * @param req Pointer to received ADU data (MBAP header + PDU)
 * @param req_len Size of received ADU data in bytes (expected between MBAP_SIZE+1 and MBADU_TCP_SIZE_MAX)
 * @param res Pointer to response buffer (must be at least MBADU_TCP_SIZE_MAX bytes)
 *
 * @return Size of response data in bytes, or 0 if no response should be sent
 *
 * @note TCP ADU format: [transaction_id(2)][protocol_id(2)][length(2)][unit_id(1)][function_code][pdu_data...]
 * @note MBAP protocol ID is expected to be 0 (MBADU_TCP_PROT_ID) for standard Modbus TCP
 * @note MBAP length field specifies the number of following bytes (unit_id + PDU)
 * @note MBAP unit id serves the same purpose as slave address in serial Modbus. Normally 0x00 or 0xFF for TCP/IP.
 * @note Response buffer must be allocated by caller with sufficient space (at least MBADU_TCP_SIZE_MAX)
 * @note Transaction ID and Unit ID from request are echoed back in the response
 */
extern size_t mbadu_tcp_handle_req(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	uint8_t *res);

#endif /* MBADU_TCP_H_INCLUDED */
