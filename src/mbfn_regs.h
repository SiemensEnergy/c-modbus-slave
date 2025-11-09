/**
 * @file mbfn_regs.h
 * @brief Modbus register function handlers - Read/Write holding and input registers
 * @author Jonas Almås
 *
 * @details This module implements Modbus function codes for register operations.
 * Supports flexible register access methods, validation, and callback mechanisms.
 *
 * @see mbreg.h for register descriptor structure
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

#ifndef MBFN_REGS_H_INCLUDED
#define MBFN_REGS_H_INCLUDED

#include "mbdef.h"
#include "mbinst.h"
#include "mbpdu.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Reads the value of one or multiple Modbus registers
 *
 * Implements Modbus function codes 0x03 (Read Holding Registers) and 0x04 (Read Input Registers).
 * If multiple registers are requested and some don't exist, missing registers are filled
 * with placeholder data (0x5555) as long as the first register exists.
 *
 * @param inst Modbus instance containing configuration
 * @param regs Array of register descriptors
 * @param n_regs Number of register descriptors in the array
 * @param req Pointer to the request PDU (function_code + start_addr + quantity)
 * @param req_len Length of the request PDU (expected exactly 5 bytes)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - response data populated
 * @retval MB_DEV_FAIL Invalid parameters
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format or size
 * @retval MB_ILLEGAL_DATA_ADDR First register address not found or read error
 *
 * @note Request format: [function_code][start_addr_hi][start_addr_lo][quantity_hi][quantity_lo]
 * @note Response format: [function_code][byte_count][register_data...]
 * @note Maximum response size is limited by PDU data size (MBPDU_DATA_SIZE_MAX)
 * @note Word order swapping is applied for input registers when inst->swap_words is set
 */
extern enum mbstatus_e mbfn_read_regs(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

/**
 * @brief Writes a single Modbus register
 *
 * Implements Modbus function code 0x06 (Write Single Register).
 * Validates the register exists and is writable before performing the write operation.
 *
 * @param inst Modbus instance containing configuration
 * @param regs Array of register descriptors (holding registers only)
 * @param n_regs Number of register descriptors in the array
 * @param req Pointer to the request PDU (function_code + address + value)
 * @param req_len Length of the request PDU (expected exactly 5 bytes)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - register written and response prepared
 * @retval MB_DEV_FAIL Invalid parameters
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format or size
 * @retval MB_ILLEGAL_DATA_ADDR Register not found, not writable, or write failed
 *
 * @note Request format: [function_code][addr_hi][addr_lo][value_hi][value_lo]
 * @note The response echoes the request data
 * @note Calls post_write_cb callback if defined on the register
 * @note Calls commit_regs_write_cb callback if defined on the instance
 */
extern enum mbstatus_e mbfn_write_reg(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

/**
 * @brief Writes multiple consecutive Modbus registers
 *
 * Implements Modbus function code 0x10 (Write Multiple Registers).
 * Performs a two-phase operation: first validates all registers can be written,
 * then performs the actual write operations. This ensures atomicity.
 *
 * @param inst Modbus instance containing configuration
 * @param regs Array of register descriptors (holding registers only)
 * @param n_regs Number of register descriptors in the array
 * @param req Pointer to the request PDU (function_code + start_addr + quantity + byte_count + data)
 * @param req_len Length of the request PDU (expected >= 6 + byte_count)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - all registers written and response prepared
 * @retval MB_DEV_FAIL Invalid parameters
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format, size, or byte count mismatch
 * @retval MB_ILLEGAL_DATA_ADDR Register not found, not writable, or write failed
 *
 * @note Request format: [function_code][start_addr_hi][start_addr_lo][quantity_hi][quantity_lo][byte_count][data...]
 * @note Response format: [function_code][start_addr_hi][start_addr_lo][quantity_hi][quantity_lo]
 * @note Calls post_write_cb callback if defined on the register
 * @note Calls commit_regs_write_cb callback if defined on the instance (Called once after all writes)
 */
extern enum mbstatus_e mbfn_write_regs(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);
/**
 * @brief Write to a register using a mask and a value
 *
 * Implements Modbus function 0x16 (Mask Write Register).
 *
 * @param inst Modbus instance containing configuration
 * @param regs Array of register descriptors (holding registers only)
 * @param n_regs Number of register descriptors in the array
 * @param req Pointer to the request PDU (function_code + address + and_mask + or_mask)
 * @param req_len Length of the request PDU (expected == 7)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - register written and response prepared
 * @retval MB_DEV_FAIL Invalid parameters
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format or size
 * @retval MB_ILLEGAL_DATA_ADDR Register not found, not writable, or write failed
 *
 * @note Request format: [function_code][addr_hi][addr_lo][and_mask_hi][and_mask_lo][or_mask_hi][or_mask_lo]
 * @note The response echoes the request data
 * @note Calls post_write_cb callback if defined on the register
 * @note Calls commit_regs_write_cb callback if defined on the instance
 */
extern enum mbstatus_e mb_fn_mask_write_reg(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

/**
 * @brief Reads and writes multiple Modbus holding registers in a single operation
 *
 * Implements Modbus function code 0x17 (Read/Write Multiple Registers).
 * This function first performs the write operation, then the read operation,
 * returning only the read data in the response.
 *
 * @param inst Modbus instance containing configuration
 * @param regs Array of register descriptors (holding registers only)
 * @param n_regs Number of register descriptors in the array
 * @param req Pointer to the request PDU (function_code + read_addr + read_qty + write_addr + write_qty + byte_count + data)
 * @param req_len Length of the request PDU (expected >= 10 + write_byte_count)
 * @param res Pointer to the response PDU structure to populate
 *
 * @retval MB_OK Success - write completed and read data prepared in response
 * @retval MB_DEV_FAIL Invalid parameters
 * @retval MB_ILLEGAL_DATA_VAL Invalid request format, size, or byte count mismatch
 * @retval MB_ILLEGAL_DATA_ADDR Register not found, not readable/writable, or operation failed
 *
 * @note Request format: [function_code][read_start_hi][read_start_lo][read_qty_hi][read_qty_lo][write_start_hi][write_start_lo][write_qty_hi][write_qty_lo][byte_count][write_data...]
 * @note Response format: [function_code][byte_count][read_data...]
 * @note Write operation is performed first, then read operation
 * @note Only read data is returned in the response
 * @note Calls post_write_cb and commit_regs_write_cb callbacks for write operations
 * @note Both read and write operations must target holding registers
 */
extern enum mbstatus_e mbfn_read_write_regs(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

#endif /* MBFN_REGS_H_INCLUDED */
