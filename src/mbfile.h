/**
 * @file mbfile.h
 * @brief Modbus File - Structure and functions for file record handling
 * @author Jonas Almås
 *
 * @details This module implements Modbus file record functionality (function
 * codes 0x14 and 0x15) by defining file descriptor structures and operations
 * for organizing register data into logical file-like structures. File records
 * provide an alternative to standard register access patterns, allowing complex
 * data organization with file numbers and record-based addressing.
 *
 * @see mbreg.h for register descriptor structures used within file records
 * @see mbinst.h for integration with Modbus slave instances
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

#ifndef MBFILE_H_INCLUDED
#define MBFILE_H_INCLUDED

#include "mbreg.h"
#include "mbpdu.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Modbus file descriptor for file record operations
 *
 * Describes a single Modbus file that contains an array of register records.
 * Files provide a hierarchical organization method where each file is identified
 * by a file number and contains multiple register records that can be accessed
 * individually using record numbers and lengths.
 *
 * @note All files in an array must be sorted by file number in ascending order for binary search
 * @note Used with Modbus function codes 0x14 (Read File Record) and 0x15 (Write File Record)
 */
struct mbfile_desc_s {
	/**
	 * @brief Modbus file number identifier
	 *
	 * Unique identifier for this file within the Modbus address space.
	 * Used by masters to specify which file to access in file record operations.
	 *
	 * @note Must be unique within the file descriptor array
	 */
	uint16_t file_no;

	/**
	 * @brief Array of register record descriptors within this file
	 *
	 * Points to an array of register descriptors that define the data
	 * records contained within this file. Each record can have different
	 * data types, sizes, and access permissions.
	 *
	 * @note Must be sorted by register address in ascending order for binary search
	 */
	const struct mbreg_desc_s *records;

	/**
	 * @brief Number of register records in this file
	 *
	 * Specifies the count of register record descriptors in the records array.
	 */
	size_t n_records;
};

enum mbfile_read_status_e {
	MBFILE_READ_OK,
	MBFILE_READ_ILLEGAL_ADDR,
	MBFILE_READ_DEVICE_ERR,
};

/**
 * @brief Find a file descriptor by file number
 *
 * Searches through an array of file descriptors to locate the descriptor
 * matching the specified file number. Uses binary search for efficient lookup,
 * requiring the file array to be sorted in ascending order by file number.
 *
 * @param files Pointer to array of file descriptors to search
 * @param n_files Number of file descriptors in the array
 * @param file_no Modbus file number to search for
 *
 * @return Pointer to matching file descriptor, or NULL if not found
 *
 * @note The files array must be sorted by file_no in ascending order
 * @note Complexity: O(log n) due to binary search algorithm
 */
extern const struct mbfile_desc_s *mbfile_find(
	const struct mbfile_desc_s *files,
	size_t n_files,
	uint16_t file_no);

/**
 * @brief Read data from a file record
 *
 * @param file Pointer to the file descriptor containing the target record
 * @param record_no Starting record number within the file (0-based index)
 * @param record_length Number of 16-bit registers to read from the record
 * @param res Pointer to response buffer structure to populate with read data
 *
 * @retval MBFILE_READ_OK Success - data copied to response buffer
 * @retval MBFILE_READ_ILLEGAL_ADDR Invalid record number or length
 * @retval MBFILE_READ_DEVICE_ERR Internal error during read operation
 *
 * @note The response buffer (res) must have sufficient space for the requested data
 */
extern enum mbfile_read_status_e mbfile_read(
	const struct mbfile_desc_s *file,
	uint16_t record_no,
	uint16_t record_length,
	struct mbpdu_buf_s *res);

/**
 * @brief Validate whether a file record write operation is allowed
 *
 * Checks if the specified file record write operation can be performed.
 * This function performs validation without actually writing data, making it
 * useful for pre-validation in multi-step write operations that require atomicity.
 *
 * @param file Pointer to the file descriptor containing the target record
 * @param record_no Starting record number within the file (0-based index)
 * @param record_length Number of 16-bit registers to write to the record
 * @param val Pointer to the data values to be written (for validation purposes)
 */
extern enum mbstatus_e mbfile_write_allowed(
	const struct mbfile_desc_s *file,
	uint16_t record_no,
	uint16_t record_length,
	const uint8_t *val);

/**
 * @brief Write data to a file record
 *
 * Writes register data to the specified record within a file descriptor.
 * Handles multi-register writes and data type conversions as needed.
 *
 * @param file Pointer to the file descriptor containing the target record
 * @param record_no Starting record number within the file (0-based index)
 * @param record_length Number of 16-bit registers to write to the record
 * @param val Pointer to the data values to write (in big endian)
 *
 * @note Calls post_write_cb callbacks if defined on the target registers
 */
extern enum mbstatus_e mbfile_write(
	const struct mbfile_desc_s *file,
	uint16_t record_no,
	uint16_t record_length,
	const uint8_t *val);

#endif /* MBFILE_H_INCLUDED */
