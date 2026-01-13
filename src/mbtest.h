/**
 * @file mbtest.h
 * @brief Modbus Test Support - Validation functions for user configuration
 * @author Jonas Almås
 *
 * @details This module provides comprehensive validation functions to help
 * unit test applications using this Modbus library. Validates descriptor
 * arrays for proper ordering, access methods, data types, and configuration
 * consistency.
 *
 * @see mbcoil.h for coil descriptor structure
 * @see mbreg.h for register descriptor structure
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

#ifndef MBTEST_H_INCLUDED
#define MBTEST_H_INCLUDED

#include "mbcoil.h"
#include "mbfile.h"
#include "mbreg.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Check that all coils are in ascending order
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_asc(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr);

/**
 * @brief Check if all coil accesses are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_valid_access(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr);

/**
 * @brief Check if all coil bit indices are within valid range [0-7]
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_valid_bit_index(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr);

/**
 * @brief Check that no coils have duplicate addresses
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_no_duplicates(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr);

/**
 * @brief Comprehensive coil validation - runs all coil tests
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_validate_all(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr);

/**
 * @brief Check that all registers are in ascending order
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_asc(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr);

/**
 * @brief Check that all register sizes are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_valid_size(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr);

/**
 * @brief Check that no registers overlap
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_dont_overlap(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr);

	/**
 * @brief Check if all register data types are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_valid_data_type(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr);

/**
 * @brief Check if all register accesses are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_valid_access(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr);

/**
 * @brief Check if all register block accesses are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_valid_block_access(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr);

/**
 * @brief Comprehensive register validation - runs all register tests
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_validate_all(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr);

/**
 * @brief Check that all file numbers are ascending and that there are no duplicates
 * @param issue_file_no Optional - e.g. printf("0x%04x\n", issue_file_no);
 * @return 1 success, 0 failure
 *
 * @note Assumes files are in ascending order
 */
extern int mbtest_files_asc_and_unique(
	const struct mbfile_desc_s *files,
	size_t n_files,
	uint16_t *issue_file_no);

#endif /* MBTEST_H_INCLUDED */
