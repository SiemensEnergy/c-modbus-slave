/**
 * @file mbfn_coils.h
 * @brief Modbus coil function handlers - Read/Write coils and discrete inputs
 * @author Jonas Almås
 *
 * @details This module implements Modbus function codes for coil operations.
 * Supports flexible coil access methods, validation, and callback mechanisms.
 *
 * @see mbcoil.h for coil descriptor structure
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

#ifndef MBFN_COILS_H_INCLUDED
#define MBFN_COILS_H_INCLUDED

#include "mbdef.h"
#include "mbcoil.h"
#include "mbinst.h"
#include "mbpdu.h"
#include <stddef.h>
#include <stdint.h>

enum {
	MBCOIL_ON=0xFF00u,
	MBCOIL_OFF=0x0000u,
};

/**
 * @brief Reads the value of one or multiple Modbus coils
 *
 * Implements Modbus function codes 0x01 (Read Coils) and 0x02 (Read Discrete Inputs).
 * If multiple coils are requested and some don't exist, missing coils are filled
 * with placeholder data (0) as long as the first coil exists.
 *
 * @param inst Modbus instance containing configuration
 * @param coils Array of coil descriptors
 * @param n_coils Number of coil descriptors in the array
 * @param req Pointer to the request PDU (function_code + start_addr + quantity)
 * @param req_len Length of the request PDU (expected exactly 5 bytes)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - response data populated
 * @retval MB_DEV_FAIL Invalid parameters (NULL pointers)
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format or size
 * @retval MB_ILLEGAL_DATA_ADDR First coil address not found or read error
 *
 * @note Request format: [function_code][start_addr_hi][start_addr_lo][quantity_hi][quantity_lo]
 * @note Response format: [function_code][byte_count][coil_data_bytes...]
 * @note Coils are packed 8 per byte, LSB first
 * @note Maximum response size is limited by PDU data size
 */
extern enum mbstatus_e mbfn_read_coils(
	const struct mbinst_s *inst,
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

/**
 * @brief Writes a single Modbus coil
 *
 * Implements Modbus function code 0x05 (Write Single Coil).
 * Validates the coil exists and is writable before performing the write operation.
 *
 * @param inst Modbus instance containing configuration
 * @param coils Array of coil descriptors
 * @param n_coils Number of coil descriptors in the array
 * @param req Pointer to the request PDU (function_code + address + value)
 * @param req_len Length of the request PDU (expected exactly 5 bytes)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - coil written and response prepared
 * @retval MB_DEV_FAIL Invalid parameters (NULL pointers)
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format, size, or coil value
 * @retval MB_ILLEGAL_DATA_ADDR Coil not found, not writable, or write failed
 *
 * @note Request format: [function_code][addr_hi][addr_lo][value_hi][value_lo]
 * @note Request value must be 0x0000 (OFF) or 0xFF00 (ON)
 * @note The response echoes the request data
 * @note Calls post_write_cb callback if defined on the coil
 * @note Calls commit_coils_write_cb callback if defined on the instance
 */
extern enum mbstatus_e mbfn_write_coil(
	const struct mbinst_s *inst,
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

/**
 * @brief Writes multiple Modbus coils
 *
 * Implements Modbus function code 0x0F (Write Multiple Coils).
 * Performs a two-phase operation: first validates all coils can be written,
 * then performs the actual write operations. This ensures atomicity.
 *
 * @param inst Modbus instance containing configuration
 * @param coils Array of coil descriptors
 * @param n_coils Number of coil descriptors in the array
 * @param req Pointer to the request PDU (function_code + start_addr + quantity + byte_count + data)
 * @param req_len Length of the request PDU (expected >= 6 + byte_count)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - all coils written and response prepared
 * @retval MB_DEV_FAIL Invalid parameters (NULL pointers)
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format, size, or byte count mismatch
 * @retval MB_ILLEGAL_DATA_ADDR Coil not found, not writable, or write failed
 *
 * @note Request format: [function_code][start_addr_hi][start_addr_lo][quantity_hi][quantity_lo][byte_count][coil_data...]
 * @note Response format: [function_code][start_addr_hi][start_addr_lo][quantity_hi][quantity_lo]
 * @note Coils are packed 8 per byte, LSB first in the request data
 * @note Calls post_write_cb for each coil that defines it
 * @note Calls commit_coils_write_cb once after all writes complete successfully
 */
extern enum mbstatus_e mbfn_write_coils(
	const struct mbinst_s *inst,
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

#endif /* MBFN_COILS_H_INCLUDED */
