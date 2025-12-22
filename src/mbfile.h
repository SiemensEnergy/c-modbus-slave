/**
 * @file mbfile.h
 * @brief Modbus File - Structure and functions for file record handling
 * @author Jonas Almås
 *
 * @details This module defines the file descriptor structured and functions
 * for operating on it.
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
 * @brief Modbus file descriptor
 *
 * @note All files in an array must be sorted by file number in ascending order
 */
struct mbfile_desc_s {
	/**
	 * @brief Modbus file number
	 *
	 * @note Valid range: [0,9999] or [0x0000,0x270F]
	 */
	uint16_t file_no;

	const struct mbreg_desc_s *records;
	size_t n_records;
};

enum mbfile_read_status_e {
	MBFILE_READ_OK,
	MBFILE_READ_ILLEGAL_ADDR,
	MBFILE_READ_DEVICE_ERR,
};

extern const struct mbfile_desc_s *mbfile_find(
	const struct mbfile_desc_s *files,
	size_t n_files,
	uint16_t file_no);

extern enum mbfile_read_status_e mbfile_read(
	const struct mbfile_desc_s *file,
	uint16_t record_no,
	uint16_t record_length,
	struct mbpdu_buf_s *res);

#endif /* MBFILE_H_INCLUDED */
