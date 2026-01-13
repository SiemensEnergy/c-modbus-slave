/**
 * @file mbfn_diag.h
 * @brief Modbus diagnostic function handlers
 * @author Jonas Almås
 *
 * @details This module implements Modbus diagnostic and communication monitoring
 * function codes that provide statistical and operational information about the
 * Modbus slave device.
 *
 * @see mbinst.h for instance configuration and internal state
 * @see mbpdu.h for protocol data unit handling
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

#ifndef MBFN_DIAG_H_INCLUDED
#define MBFN_DIAG_H_INCLUDED

#include "mbinst.h"
#include "mbpdu.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Handles Modbus diagnostic function requests
 *
 * Implements Modbus function code 0x08 (Diagnostics) which provides access to
 * various diagnostic and testing functions. This function code uses subfunctions
 * to specify the particular diagnostic operation to perform. Common subfunctions
 * include echo tests, counter resets, status queries, and other device diagnostics.
 *
 * @param inst Modbus instance containing configuration and diagnostic state
 * @param req Pointer to the request PDU (function_code + subfunction + data)
 * @param req_len Length of the request PDU (minimum 3 bytes: function + subfunction)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - diagnostic operation completed and response prepared
 * @retval MB_DEV_FAIL Invalid parameters (NULL pointers)
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format, length, or unsupported subfunction
 * @retval MB_ILLEGAL_FUNC Diagnostic function not supported or disabled
 *
 * @note Request format: [function_code][subfunction_hi][subfunction_lo][data...]
 * @note Response format varies by subfunction, typically echoes request format
 */
extern enum mbstatus_e mbfn_diag(
	struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

/**
 * @brief Returns the communication event counter for diagnostic purposes
 *
 * Implements Modbus function code 0x0B (Get Comm Event Counter) which returns
 * the current communication event counter value. This counter is automatically
 * incremented by the library for each successful Modbus message processed and
 * provides a simple metric for monitoring communication activity.
 *
 * @param inst Modbus instance containing the communication event counter state
 * @param req Pointer to the request PDU (function_code + subfunction + data)
 * @param req_len Length of the request PDU (minimum 3 bytes: function + subfunction)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - event counter returned in response
 * @retval MB_DEV_FAIL Invalid parameters (NULL pointers)
 *
 * @note This function does not require any request data beyond the function code
 * @note Response format: [function_code][status_hi][status_lo][event_count_hi][event_count_lo]
 * @note This is a serial-only function (RTU/ASCII), not used in TCP/IP
 */
extern enum mbstatus_e mbfn_comm_event_counter(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

/**
 * @brief Returns the communication event log for diagnostic purposes
 *
 * Implements Modbus function code 0x0C (Get Comm Event Log) which returns
 * a detailed log of communication events and diagnostic information. This
 * function provides comprehensive diagnostic data including status information,
 * event counters, and a log of recent communication events.
 *
 * @param inst Modbus instance containing the communication event log state
 * @param req Pointer to the request PDU (function_code + subfunction + data)
 * @param req_len Length of the request PDU (minimum 3 bytes: function + subfunction)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - communication event log returned in response
 * @retval MB_DEV_FAIL Invalid parameters (NULL pointers)
 *
 * @note This function does not require any request data beyond the function code
 * @note Response format: [function_code][byte_count][status_hi][status_lo][event_count_hi][event_count_lo][message_count_hi][message_count_lo][events...]
 * @note This is a serial-only function (RTU/ASCII), not used in TCP/IP
 * @note The actual event log implementation may vary based on device requirements
 */
extern enum mbstatus_e mbfn_comm_event_log(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

#endif /* MBFN_DIAG_H_INCLUDED */
