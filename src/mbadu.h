/**
 * @file mbadu.h
 * @brief Modbus Application Data Unit (Serial) - ADU wrapper with slave address and CRC
 * @author Jonas Almås
 *
 * @details This module provides serial Modbus Application Data Unit handling.
 * It wraps the Protocol Data Unit (PDU) with slave address and CRC-16 checksum
 * for use in Modbus RTU serial communications.
 *
 * @note Format: [slave_addr][PDU][crc16] - Used for Modbus RTU (RS485/RS232)
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

#ifndef MBADU_H_INCLUDED
#define MBADU_H_INCLUDED

#include "mbinst.h"
#include <stddef.h>
#include <stdint.h>

enum {
	/**
	 * - 1 Slave address
	 * - 1 Function code (Technically not part of the ADU, but required)
	 * - 2 crc
	 */
	MBADU_SIZE_MIN=4u,
	/*
	 * - 1 Slave address
	 * - 253 PDU
	 *   - 1 Function code
	 *   - 252 PDU data
	 * - 2 crc
	 */
	MBADU_SIZE_MAX=256u,
};

enum {
	MBADU_SLAVE_ADDR_MIN=1u,
	MBADU_SLAVE_ADDR_MAX=247u,

	/**
	 * Modbus broadcast slave address. All slaves shall act, but not reply.
	 */
	MBADU_ADDR_BROADCAST = 0u,
	/**
	 * @brief Broadcast with response (used for identifying devices)
	 * @note This is not standard, but nice to have
	 */
	MBADU_ADDR_DEFAULT_RESP = 248u,
};

/**
 * @brief Handle Modbus ADU request
 *
 * Processes a complete Modbus Application Data Unit for serial communication (RTU mode).
 * Validates slave address, performs CRC checking, extracts the PDU, processes the request,
 * and generates a properly formatted response with CRC.
 *
 * @param inst Pointer to the Modbus instance containing coil/register maps and configuration
 * @param req Pointer to received ADU data (slave_addr + PDU + CRC)
 * @param req_len Size of received ADU data in bytes (expected between MBADU_SIZE_MIN and MBADU_SIZE_MAX)
 * @param res Pointer to response buffer (must be at least MBADU_SIZE_MAX bytes)
 *
 * @return Size of response data in bytes, or 0 if no response should be sent
 *
 * @note ADU format: [slave_addr][function_code][pdu_data...][crc_lo][crc_hi]
 * @note Responds to slave_addr, broadcast address (0), and optionally address 248 if enabled
 * @note Broadcast requests (address 0) never generate responses
 * @note Returns 0 for invalid requests, CRC errors, or broadcast messages
 * @note Response buffer must be allocated by caller with sufficient space (MBADU_SIZE_MAX)
 */
extern size_t mbadu_handle_req(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	uint8_t *res);

#endif /* MBADU_H_INCLUDED */
