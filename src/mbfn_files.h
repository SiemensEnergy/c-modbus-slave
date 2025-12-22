/**
 * @file mbfn_files.h
 * @brief Modbus files function handles - Read/Write file records
 * @author Jonas Almås
 *
 * @details This module implements Modbus function codes for file record operations.
 *
 * @see mbfile.h for file descriptor structure
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

#ifndef MBFN_FILES_H_INCLUDED
#define MBFN_FILES_H_INCLUDED

#include "mbdef.h"
#include "mbinst.h"
#include "mbpdu.h"
#include <stddef.h>
#include <stdint.h>

extern enum mbstatus_e mbfn_file_read(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

extern enum mbstatus_e mbfn_file_write(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res);

#endif /* MBFN_FILES_H_INCLUDED */
