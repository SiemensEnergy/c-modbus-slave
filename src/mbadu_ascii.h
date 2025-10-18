/**
 * @file mbadu_ascii.h
 * @brief Modbus Ascii Application Data Unit (Serial)
 * @author Jonas Almås
 *
 * @note Format: [:][slave_addr-2][PDU][lrc-2][CR/LF]
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

#ifndef MBADU_ASCII_H_INCLUDED
#define MBADU_ASCII_H_INCLUDED

#include "mbinst.h"
#include <stddef.h>
#include <stdint.h>

enum {
	/**
	 * - 1 Start char
	 * - 2 Slave address
	 * - N PDU
	 * - 2 lrc
	 * - 2 cr/lf
	 */
	MBADU_ASCII_HEADER_SIZE=7u,

	/**
	 * - 1 Start char
	 * - 2 Slave address
	 * - 2 Function code (Technically not part of the ADU, but required)
	 * - 2 lrc
	 * - 2 cr/lf
	 */
	MBADU_ASCII_SIZE_MIN = MBADU_ASCII_HEADER_SIZE + 2u,
	/*
	 * - 1 Start char
	 * - 2 Slave address
	 * - 253*2 PDU
	 *   - 1*2 Function code
	 *   - 252*2 PDU data
	 * - 2 lrc
	 * - 2 crlf
	 */
	MBADU_ASCII_SIZE_MAX = MBADU_ASCII_HEADER_SIZE + MBPDU_SIZE_MAX*2u,

	MBADU_ASCII_START_CHAR=':',
};

/**
 * @brief Handle Modbus ASCII ADU request
 *
 * Processes a complete Modbus Application Data Unit for ASCII serial communication.
 * Validates the ASCII frame format, checks slave address, performs LRC checking,
 * extracts and converts the ASCII-encoded PDU to binary, processes the request,
 * and generates a properly formatted ASCII response with LRC.
 *
 * @param inst Pointer to the Modbus instance containing coil/register maps and configuration
 * @param req Pointer to received ASCII ADU data ([:][slave_addr-2][PDU][lrc-2][CR/LF])
 * @param req_len Size of received ASCII ADU data in bytes (expected between MBADU_ASCII_SIZE_MIN and MBADU_ASCII_SIZE_MAX)
 * @param res Pointer to response buffer (must be at least MBADU_ASCII_SIZE_MAX bytes)
 *
 * @return Size of response data in bytes, or 0 if no response should be sent
 *
 * @note ASCII ADU format: [':'][slave_addr_hex][function_code_hex][pdu_data_hex...][lrc_hex][CR][LF]
 * @note Responds to slave_addr, broadcast address (0), and optionally address 248 if enabled
 * @note Broadcast requests (address 0) never generate responses
 * @note Returns 0 for invalid requests, LRC errors, format errors, or broadcast messages
 * @note Response buffer must be allocated by caller with sufficient space (MBADU_ASCII_SIZE_MAX)
 * @note All data except start character and CR/LF is encoded as ASCII hexadecimal pairs
 */
extern size_t mbadu_ascii_handle_req(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	uint8_t *res);

#endif /* MBADU_ASCII_H_INCLUDED */
