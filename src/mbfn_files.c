/**
 * @file mbfn_files.c
 * @brief Implementation of Modbus file records function handles
 * @author Jonas Almås
 *
 * MISRA Deviations:
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

#include "mbfn_files.h"
#include "endian.h"
#include "mbfile.h"
#include "mbpdu.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
	/**
	 * Function code (1 byte)
	 * Byte count (1 byte)
	 */
	READ_REQ_HEADER_SIZE=2u,

	/**
	 * Reference type (1 byte)
	 * File number (2 bytes)
	 * Record number (2 bytes)
	 * Record length (2 bytes)
	 */
	READ_SUB_REQ_SIZE=7u,

	READ_REQ_MIN_SIZE = READ_REQ_HEADER_SIZE + READ_SUB_REQ_SIZE,

	/**
	 * (MBPDU_DATA_SIZE_MAX - READ_REQ_HEADER_SIZE) - ((MBPDU_DATA_SIZE_MAX - READ_REQ_HEADER_SIZE) % SUB_REQ_SIZE)
	 *
	 * (252 - 2) - ((252 - 2) % 7)
	 */
	READ_REQ_MAX_BYTE_COUNT=0xF5,
};

enum {
	READ_SUB_REQ_REF_TYPE_POS=0u,
	READ_SUB_REQ_FILE_NO_POS=1u,
	READ_SUB_REQ_REC_NO_POS=3u,
	READ_SUB_REQ_REC_LEN_POS=5u,
};

enum {
	/**
	 * Function code (1 byte)
	 * Byte count (1 byte)
	 */
	READ_RESP_HEADER_SIZE=2u,
	READ_SUB_RESP_HEADER_SIZE=2u,
	READ_RESP_MAX_BYTE_COUNT=0xF5,
};

enum {
	READ_SUB_RESP_LEN_POS=0u,
	READ_SUB_RESP_REF_TYPE_POS=1u,
};

enum {
	/**
	 * Function code (1 byte)
	 * Byte count (1 byte)
	 */
	WRITE_REQ_HEADER_SIZE=2u,

	/**
	 * Reference type (1 byte)
	 * File number (2 bytes)
	 * Record number (2 bytes)
	 * Record length (2 bytes)
	 */
	WRITE_SUB_REQ_HEADER_SIZE=7u,

	/**
	 * Header (7 byte)
	 * Record data (>= 2 bytes)
	 */
	WRITE_SUB_REQ_MIN_SIZE=WRITE_SUB_REQ_HEADER_SIZE + 2u,

	WRITE_REQ_MIN_SIZE = WRITE_REQ_HEADER_SIZE + WRITE_SUB_REQ_MIN_SIZE,

	WRITE_REQ_MAX_BYTE_COUNT = MBPDU_DATA_SIZE_MAX - WRITE_REQ_HEADER_SIZE,
};

enum {
	WRITE_SUB_REQ_REF_TYPE_POS=0u,
	WRITE_SUB_REQ_FILE_NO_POS=1u,
	WRITE_SUB_REQ_REC_NO_POS=3u,
	WRITE_SUB_REQ_REC_LEN_POS=5u,
};

enum {REF_TYPE=0x06u};
enum {MAX_REC_NO=0x270Fu};

extern enum mbstatus_e mbfn_file_read(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	uint8_t byte_count;
	size_t resp_byte_count;
	size_t i, n_sub_reqs;
	const uint8_t *p;
	uint16_t file_no, record_no, record_length;
	const struct mbfile_desc_s *file;

	if ((inst==NULL) || (req==NULL) || (res==NULL)) return MB_DEV_FAIL;
	if (req[0]!=MBFC_READ_FILE_RECORD) return MB_DEV_FAIL;

	if (req_len < READ_REQ_MIN_SIZE) {
		return MB_ILLEGAL_DATA_VAL;
	}

	byte_count = req[1];

	if ((byte_count < READ_SUB_REQ_SIZE)
			|| (byte_count > READ_REQ_MAX_BYTE_COUNT)
			|| (byte_count != (req_len-READ_REQ_HEADER_SIZE))
			|| ((byte_count % READ_SUB_REQ_SIZE) != 0)) {
		return MB_ILLEGAL_DATA_VAL;
	}

	n_sub_reqs = byte_count / READ_SUB_REQ_SIZE;

	/* Validate all sub-requests */
	resp_byte_count = 0u;
	for (i=0u; i<n_sub_reqs; ++i) {
		p = req + READ_REQ_HEADER_SIZE + (i*READ_SUB_REQ_SIZE);

		if (p[READ_SUB_REQ_REF_TYPE_POS] != REF_TYPE) {
			return MB_ILLEGAL_DATA_VAL;
		}

		file_no = betou16(p + READ_SUB_REQ_FILE_NO_POS);
		record_no = betou16(p + READ_SUB_REQ_REC_NO_POS);
		record_length = betou16(p + READ_SUB_REQ_REC_LEN_POS);

		if (file_no==0u) { /* Range: (0x0000,0xFFFF] */
			return MB_ILLEGAL_DATA_ADDR;
		}
		if (!inst->allow_ext_file_recs
				&& (record_no>MAX_REC_NO)) { /* Range: [0,0x270F] */
			return MB_ILLEGAL_DATA_ADDR;
		}
		if (record_length==0u) {
			return MB_ILLEGAL_DATA_VAL;
		}

		resp_byte_count += READ_SUB_RESP_HEADER_SIZE + (record_length * 2u);
	}

	if (resp_byte_count > READ_RESP_MAX_BYTE_COUNT) {
		return MB_ILLEGAL_DATA_VAL;
	}

	res->p[1] = (uint8_t)resp_byte_count;
	res->size = 2u;

	for (i=0u; i<n_sub_reqs; ++i) {
		p = req + READ_REQ_HEADER_SIZE + (i*READ_SUB_REQ_SIZE);

		file_no = betou16(p + READ_SUB_REQ_FILE_NO_POS);
		record_no = betou16(p + READ_SUB_REQ_REC_NO_POS);
		record_length = betou16(p + READ_SUB_REQ_REC_LEN_POS);

		file = mbfile_find(inst->files, inst->n_files, file_no);
		if (file==NULL) {
			return MB_ILLEGAL_DATA_ADDR;
		}

		res->p[res->size + READ_SUB_RESP_LEN_POS] = (uint8_t)(1u + (record_length * 2u)); /* File resp. length */
		res->p[res->size + READ_SUB_RESP_REF_TYPE_POS] = REF_TYPE;
		res->size += READ_SUB_RESP_HEADER_SIZE;

		switch (mbfile_read(file, record_no, record_length, res)) {
		case MBFILE_READ_OK: break;
		case MBFILE_READ_ILLEGAL_ADDR: return MB_ILLEGAL_DATA_ADDR;
		case MBFILE_READ_DEVICE_ERR: return MB_DEV_FAIL;
		default: return MB_DEV_FAIL;
		}
	}

	return MB_OK;
}

extern enum mbstatus_e mbfn_file_write(
	const struct mbinst_s *inst,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	uint8_t byte_count;
	size_t remaining_bytes;
	const uint8_t *p, *base;
	uint16_t file_no, record_no, record_length;
	const struct mbfile_desc_s *file;
	enum mbstatus_e status;

	if ((inst==NULL) || (req==NULL) || (res==NULL)) return MB_DEV_FAIL;
	if (req[0]!=MBFC_WRITE_FILE_RECORD) return MB_DEV_FAIL;

	if (req_len < WRITE_REQ_MIN_SIZE) {
		return MB_ILLEGAL_DATA_VAL;
	}

	byte_count = req[1];

	if ((byte_count < WRITE_SUB_REQ_MIN_SIZE)
			|| (byte_count > WRITE_REQ_MAX_BYTE_COUNT)
			|| (byte_count != (req_len-WRITE_REQ_HEADER_SIZE))) {
		return MB_ILLEGAL_DATA_VAL;
	}

	/* Validate request and ensure all registers in all files can be written
	   to before writing anything. */
	base = req + WRITE_REQ_HEADER_SIZE;
	p = base;
	while ((p-base) < byte_count) {
		remaining_bytes = (size_t)byte_count - (size_t)(p-base);
		if (remaining_bytes < WRITE_SUB_REQ_MIN_SIZE) {
			return MB_ILLEGAL_DATA_VAL;
		}
		if (p[WRITE_SUB_REQ_REF_TYPE_POS] != REF_TYPE) {
			return MB_ILLEGAL_DATA_VAL;
		}

		file_no = betou16(p + WRITE_SUB_REQ_FILE_NO_POS);
		record_no = betou16(p + WRITE_SUB_REQ_REC_NO_POS);
		record_length = betou16(p + WRITE_SUB_REQ_REC_LEN_POS);

		if (file_no==0u) { /* Range: (0x0000,0xFFFF] */
			return MB_ILLEGAL_DATA_ADDR;
		}
		if (!inst->allow_ext_file_recs
				&& (record_no>MAX_REC_NO)) { /* Range: [0,0x270F] */
			return MB_ILLEGAL_DATA_ADDR;
		}
		if ((record_length==0u)
				|| ((record_length*2u) > (remaining_bytes-WRITE_SUB_REQ_HEADER_SIZE))) {
			return MB_ILLEGAL_DATA_VAL;
		}

		file = mbfile_find(inst->files, inst->n_files, file_no);
		if (file==NULL) {
			return MB_ILLEGAL_DATA_ADDR;
		}

		p += WRITE_SUB_REQ_HEADER_SIZE;

		status = mbfile_write_allowed(file, record_no, record_length, p);
		if (status != MB_OK) {
			return status;
		}

		p += record_length * 2u;
	}

	res->p[1] = byte_count;
	res->size = 2u;

	/* Write the actual data */
	base = req + WRITE_REQ_HEADER_SIZE;
	p = base;
	while ((p-base) < byte_count) {
		file_no = betou16(p + WRITE_SUB_REQ_FILE_NO_POS);
		record_no = betou16(p + WRITE_SUB_REQ_REC_NO_POS);
		record_length = betou16(p + WRITE_SUB_REQ_REC_LEN_POS);
		p += WRITE_SUB_REQ_HEADER_SIZE;

		file = mbfile_find(inst->files, inst->n_files, file_no);
		status = mbfile_write(file, record_no, record_length, p);
		if (status != MB_OK) { /* Request might me incomplete, not ideal... */
			return status;
		}

		/* Build response */
		res->p[res->size + WRITE_SUB_REQ_REF_TYPE_POS] = REF_TYPE;
		u16tobe(file_no, res->p + res->size + WRITE_SUB_REQ_FILE_NO_POS);
		u16tobe(record_no, res->p + res->size + WRITE_SUB_REQ_REC_NO_POS);
		u16tobe(record_length, res->p + res->size + WRITE_SUB_REQ_REC_LEN_POS);
		res->size += WRITE_SUB_REQ_HEADER_SIZE;

		memcpy(res->p + res->size, p, record_length * 2u);
		res->size += record_length * 2u;

		p += record_length * 2u;
	}

	if (inst->commit_regs_write_cb!=NULL) {
		inst->commit_regs_write_cb(inst);
	}

	return MB_OK;
}
