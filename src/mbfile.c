/**
 * @file mbfile.c
 * @brief Implementation of Modbus file record handling functions
 * @author Jonas Almås
 *
 * MISRA Deviations:
 * - Rule 13.4: The result of an assignment operator should not be used
 *   Rationale: Improves readability and code maintainability
 *   Mitigation: Parentheses used to clarify intent
 * - Rule 14.2: A for loop shall be well-formed
 *   Rationale: Complex loop conditions necessary for protocol buffer parsing
 *   Mitigation: Loop variables properly initialized and bounds checked
 * - Rule 15.5: A function should have a single point of exit at the end
 *   Rationale: Multiple returns improve readability and reduce nesting for error conditions
 *   Mitigation: Each return path clearly documented with appropriate error handling
 * - Rule 15.7: All if … else if constructs shall be terminated with an else statement
 *   Rationale: Improves readability and code maintainability
 * - Rule 18.4: The +, -, += and -= operators should not be applied to an expression of pointer type
 *   Rationale: Pointer arithmetic necessary for efficient buffer parsing and generation
 *   Mitigation: Bounds checking performed, arithmetic limited to validated buffer operations
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

#include "mbfile.h"
#include "mbreg.h"

enum {BSEARCH_THRESHOLD=16u};

extern const struct mbfile_desc_s *mbfile_find(
	const struct mbfile_desc_s *files,
	size_t n_files,
	uint16_t file_no)
{
	const struct mbfile_desc_s *file;
	size_t l, m, r;
	size_t i;

	if (!files || (n_files==0u)) return NULL;

	if (n_files > BSEARCH_THRESHOLD) { /* Only use binary search for larger descriptor sets */
		l = 0u;
		r = n_files - 1u;

		while (l <= r) {
			m = l + (r - l) / 2u;
			file = files + m;

			if (file->file_no == file_no) {
				return file;
			} else if (file->file_no < file_no) {
				l = m + 1u;
			} else {
				if (m == 0u) break; /* Prevent underflow */
				r = m - 1u;
			}
		}
	} else {
		for (i=0u; i<n_files; ++i) {
			file = files + i;
			if (file->file_no == file_no) {
				return file;
			}
		}
	}

	return NULL;
}

extern enum mbfile_read_status_e mbfile_read(
	const struct mbfile_desc_s *file,
	uint16_t record_no,
	uint16_t record_length,
	struct mbpdu_buf_s *res)
{
	uint16_t addr, reg_offs;
	const struct mbreg_desc_s *reg;
	size_t n_read_regs;

	/* If we read multiple records and one of them doesn't exist,
	   we just fill that with zero.
	   We don't want to do this if the first record is missing.
	 */
	if (!mbreg_find_desc(file->records, file->n_records, record_no)) {
		return MBFILE_READ_ILLEGAL_ADDR;
	}

	for (reg_offs=0u; reg_offs < record_length; ) {
		addr = record_no + reg_offs;
		if ((reg = mbreg_find_desc(file->records, file->n_records, addr)) != NULL) {
			n_read_regs = mbreg_read(
				reg,
				addr,
				record_length-reg_offs,
				res ? (res->p + res->size) : NULL,
				0);
			switch (n_read_regs) {
			case MBREG_READ_DEV_FAIL:
				return MBFILE_READ_DEVICE_ERR;
			case MBREG_READ_LOCKED:
			case MBREG_READ_NO_ACCESS:
				if (res!=NULL) {
					res->p[res->size] = 0x00u;
					res->p[res->size+1u] = 0x00u;
					res->size += 2u;
				}
				++reg_offs;
				break;
			default:
				if (res!=NULL) {
					res->size += n_read_regs*2u;
				}
				reg_offs += (uint16_t)n_read_regs;
				break;
			}
		} else {
			if (res!=NULL) {
				res->p[res->size] = 0x00u;
				res->p[res->size+1u] = 0x00u;
				res->size += 2u;
			}
			++reg_offs;
		}
	}

	return MBFILE_READ_OK;
}

extern enum mbstatus_e mbfile_write_allowed(
	const struct mbfile_desc_s *file,
	uint16_t record_no,
	uint16_t record_length,
	const uint8_t *val)
{
	const struct mbreg_desc_s *reg;
	uint16_t addr, reg_offs;
	size_t n_regs_written;

	for (reg_offs=0u; reg_offs<record_length; ) {
		addr = record_no + reg_offs;
		if ((reg = mbreg_find_desc(file->records, file->n_records, addr)) == NULL) {
			return MB_ILLEGAL_DATA_ADDR;
		}

		n_regs_written = mbreg_write_allowed(
			reg,
			addr,
			record_no,
			record_length-reg_offs,
			val + (reg_offs*2u));
		if (n_regs_written == 0u) {
			return MB_ILLEGAL_DATA_ADDR;
		}

		/* Advance by the actual written register size to handle
		   sub-registers correctly */
		reg_offs += (uint16_t)n_regs_written;
	}

	return MB_OK;
}

extern enum mbstatus_e mbfile_write(
	const struct mbfile_desc_s *file,
	uint16_t record_no,
	uint16_t record_length,
	const uint8_t *val)
{
	const struct mbreg_desc_s *reg;
	enum mbstatus_e status;
	uint16_t addr, reg_offs;
	size_t n_regs_written;

	for (reg_offs=0u; reg_offs<record_length; ) {
		addr = record_no + reg_offs;
		reg = mbreg_find_desc(file->records, file->n_records, addr);

		status = mbreg_write(
			reg,
			addr,
			record_length-reg_offs,
			val + (reg_offs*2u),
			&n_regs_written);
		if (status!=MB_OK) return status;
		if (n_regs_written==0u) return MB_DEV_FAIL;

		if (reg->post_write_cb!=NULL) {
			reg->post_write_cb();
		}

		/* Advance by the actual written register size to handle
		   sub-registers correctly */
		reg_offs += (uint16_t)n_regs_written;
	}

	return MB_OK;
}
