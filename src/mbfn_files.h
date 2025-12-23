/**
 * @file mbfn_files.h
 * @brief Modbus files function handlers - Read/Write file records
 * @author Jonas Almås
 *
 * @details This module implements Modbus function codes for file record operations:
 * - Function 0x14: Read File Record
 * - Function 0x15: Write File Record
 *
 * File records provide an alternative method for organizing register data into
 * logical file-like structures with file numbers and record-based addressing.
 * Each file can contain multiple records that can be accessed individually.
 *
 * @see mbfile.h for file descriptor structure
 * @see mbreg.h for register descriptor structures used within file records
 * @see mbpdu.h for protocol data unit handling
 * @see mbinst.h for Modbus slave instance integration
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

#ifndef MBFN_FILES_H_INCLUDED
#define MBFN_FILES_H_INCLUDED

#include "mbdef.h"
#include "mbinst.h"
#include "mbpdu.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Reads data from one or multiple file records
 *
 * Implements Modbus function code 0x14 (Read File Record). This function allows
 * reading data from multiple file records in a single request, where each sub-request
 * specifies a file number, record number, and record length. The function validates
 * each sub-request and returns the requested data or appropriate error responses.
 *
 * @param inst Modbus instance containing file descriptor configuration
 * @param req Pointer to the request PDU containing sub-requests
 * @param req_len Length of the request PDU in bytes
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - response data populated with file record contents
 * @retval MB_DEV_FAIL Invalid parameters or internal error
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format, length, or sub-request structure
 * @retval MB_ILLEGAL_DATA_ADDR File number not found, record not found, or read error
 *
 * @note Request format: [function_code][byte_count][sub_req1][sub_req2]...
 * @note Sub-request format: [ref_type][file_number_hi][file_number_lo][record_number_hi][record_number_lo][record_length_hi][record_length_lo]
 * @note Response format: [function_code][resp_length][sub_resp1][sub_resp2]...
 * @note Sub-response format: [file_resp_length][ref_type][record_data...]
 * @note Reference type must be 0x06 per Modbus specification
 * @note Maximum 35 sub-requests per request (limited by PDU size)
 * @note Extended file records (>9999) supported when inst->allow_ext_file_recs is enabled
 */
extern enum mbstatus_e mbfn_file_read(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

/**
 * @brief Writes data to one or multiple file records
 *
 * Implements Modbus function code 0x15 (Write File Record). This function allows
 * writing data to multiple file records in a single request, where each sub-request
 * specifies a file number, record number, record length, and the data to write.
 * The function validates each sub-request and performs the write operations.
 *
 * @param inst Modbus instance containing file descriptor configuration
 * @param req Pointer to the request PDU containing sub-requests with write data
 * @param req_len Length of the request PDU in bytes
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - all file records written and response prepared
 * @retval MB_DEV_FAIL Invalid parameters or internal error
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format, length, or sub-request structure
 * @retval MB_ILLEGAL_DATA_ADDR File number not found, record not found, not writable, or write failed
 *
 * @note Request format: [function_code][byte_count][sub_req1][sub_req2]...
 * @note Sub-request format: [ref_type][file_number_hi][file_number_lo][record_number_hi][record_number_lo][record_length_hi][record_length_lo][record_data...]
 * @note Response: Echo of the request
 * @note Reference type must be 0x06 per Modbus specification
 * @note Maximum sub-requests per request limited by PDU size and data length
 * @note Each sub-request is processed independently - partial success is possible
 * @note Calls post_write_cb callbacks if defined on the target registers
 * @note Calls commit_regs_write_cb callback if defined on the instance (called once after all writes)
 * @note Extended file records (>9999) supported when inst->allow_ext_file_recs is enabled
 */
extern enum mbstatus_e mbfn_file_write(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

#endif /* MBFN_FILES_H_INCLUDED */
