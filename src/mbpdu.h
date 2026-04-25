/**
 * @file mbpdu.h
 * @brief Modbus Protocol Data Unit (PDU) - Function code routing and request handling
 * @author Jonas Almås
 *
 * @details This module handles the protocol layer of Modbus by routing function
 * codes to the appropriate handlers. Defines PDU structure, status codes,
 * function codes, and provides the main request processing interface for all
 * Modbus operations.
 *
 * @note PDU format: [function_code][data...] - Core protocol without transport wrapper
 * @see mbinst.h for instance configuration
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

#ifndef MBPDU_H_INCLUDED
#define MBPDU_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

enum {
	MBPDU_DATA_SIZE_MAX=252u,
	/*
	 * - 1 Function code
	 */
	MBPDU_SIZE_MIN=1u,
	/*
	 * - 1 Function code
	 * - 252 PDU data
	 */
	MBPDU_SIZE_MAX=1u+MBPDU_DATA_SIZE_MAX,
};

/**
 * @brief Modbus PDU Buffer
 *
 * A buffer management structure used to build Modbus PDU responses and track
 * the amount of data written.
 *
 * The PDU data follows the standard Modbus format:
 * [function_code][pdu_data...]
 *
 * @note The buffer pointed to by 'p' must be allocated with at least
 *       MBPDU_SIZE_MAX bytes to accommodate the largest possible PDU
 * @note The 'size' field tracks bytes actually used, not buffer capacity
 * @note See MBPDU_SIZE_MAX for maximum PDU size constant
 */
struct mbpdu_buf_s {
	uint8_t *p; /**< Pointer to PDU buffer (minimum MBPDU_SIZE_MAX bytes required) */
	size_t size; /**< Number of bytes currently used in the buffer */
};

struct mbinst_s; /* Forward declaration, see "mbinst.h" */

/**
 * @brief Handle Modbus PDU request
 *
 * Processes a Modbus Protocol Data Unit by routing the function code to the
 * appropriate handler and generating a response. Supports all standard Modbus
 * function codes for reading/writing coils and registers, with fallback to
 * custom function handlers.
 *
 * @param inst Pointer to the Modbus instance containing coil/register maps and configuration
 * @param req Pointer to the complete PDU data (function code + request data)
 * @param req_len Length of the PDU data in bytes (expected >= 1)
 * @param res Pointer to response PDU data to populate (must be at least MBPDU_SIZE_MAX bytes)
 *
 * @return Size of response data in bytes, or 0 if no response should be sent
 *
 * @note PDU format: [function_code][request_data...]
 * @note On error, sets error flag (0x80) in response function code and includes error code
 * @note Calls custom function handler 'handle_fn_cb' (if present) for unsupported or missing function codes
 * @note Response PDU is always populated, even on errors
 */
extern size_t mbpdu_handle_req(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	uint8_t *res);

#endif /* MBPDU_H_INCLUDED */
