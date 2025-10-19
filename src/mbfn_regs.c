/**
 * @file mbfn_regs.c
 * @brief Implementation of Modbus register function handlers
 * @author Jonas Almås
 *
 * MISRA Deviations:
 * - Rule 12.1: The precedence of operators within expressions should be made explicit
 * - Rule 12.3: The comma operator should not be used
 * - Rule 13.4: The result of an assignment operator should not be used
 * - Rule 14.2: A for loop shall be well-formed
 * - Rule 14.4: The controlling expression of an if statement and the controlling expression of an iteration-statement shall have essentially Boolean type
 * - Rule 15.5: A function should have a single point of exit at the end
 * - Rule 18.4: The +, -, += and -= operators should not be applied to an expression of pointer type
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

#include "mbfn_regs.h"
#include "endian.h"
#include "mbreg.h"
#include <stddef.h>
#include <stdint.h>

enum {
	MBREG_N_READ_MAX=0x7Du,
	MBREG_N_WRITE_MAX=0x7Bu,
};

static enum mbstatus_e read_regs(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t start_addr,
	uint16_t n_req_regs,
	struct mbpdu_buf_s *res,
	int is_hold_reg)
{
	const struct mbreg_desc_s *reg;
	uint16_t addr, reg_offs;
	size_t n_read_regs;

	if ((n_req_regs==0u) || (n_req_regs>MBREG_N_READ_MAX)) {
		return MB_ILLEGAL_DATA_VAL;
	}

	/* If we read multiple registers and one of them doesn't exist,
	   we just fill that with some "random" data.
	   We don't want to do this if the first register is missing.
	 */
	if (!mbreg_find_desc(regs, n_regs, start_addr)) {
		return MB_ILLEGAL_DATA_ADDR;
	}

	if (res) {
		res->p[1] = (uint8_t)(2u * n_req_regs); /* Byte count */
		res->size = 2u;
	}

	/* Read register value into response data */
	for (reg_offs=0u; reg_offs < n_req_regs; ) {
		addr = start_addr + reg_offs;
		if ((reg = mbreg_find_desc(regs, n_regs, addr))) {
			n_read_regs = mbreg_read(
				reg,
				addr,
				n_req_regs-reg_offs,
				res ? (res->p + res->size) : NULL,
				inst->swap_words && !is_hold_reg);
			if (n_read_regs==MBREG_READ_DEV_FAIL) {
				return MB_DEV_FAIL;
			} else if (n_read_regs==MBREG_READ_LOCKED) {
				return MB_ILLEGAL_DATA_ADDR;
			} else if (n_read_regs!=MBREG_READ_NO_ACCESS) {
				if (res) {res->size += n_read_regs*2u;}
			}

			reg_offs += (uint16_t)n_read_regs;
		} else {
			if (res) {
				res->p[res->size] = 0x00u;
				res->p[res->size+1u] = 0x00u;
				res->size += 2u;
			}
			++reg_offs;
		}
	}

	return MB_OK;
}

static enum mbstatus_e write_regs(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t start_addr,
	uint16_t n_req_regs,
	uint8_t byte_count,
	const uint8_t *req_write_data,
	struct mbpdu_buf_s *res)
{
	const struct mbreg_desc_s *reg;
	enum mbstatus_e status;
	uint16_t reg_offs, addr;
	size_t n_regs_written;

	if ((n_req_regs==0u) || (n_req_regs>MBREG_N_WRITE_MAX)) {
		return MB_ILLEGAL_DATA_VAL;
	}

	/* Make sure received byte count matches number of registers to write */
	if ((n_req_regs*2u) != byte_count) {
		return MB_ILLEGAL_DATA_VAL;
	}

	/* Ensure all registers exist and can be written to before writing anything */
	for (reg_offs=0u; reg_offs < n_req_regs; ) {
		addr = start_addr + reg_offs;
		if (!(reg = mbreg_find_desc(regs, n_regs, addr))) {
			return MB_ILLEGAL_DATA_ADDR;
		}

		n_regs_written = mbreg_write_allowed(
			reg,
			addr,
			start_addr,
			n_req_regs-reg_offs,
			req_write_data + reg_offs*2u);
		if (n_regs_written == 0u) {
			return MB_ILLEGAL_DATA_ADDR;
		}

		/* Advance by the actual written register size to handle
		   sub-registers correctly */
		reg_offs += (uint16_t)n_regs_written;
	}

	/* Write registers */
	for (reg_offs=0u; reg_offs < n_req_regs; ) {
		addr = start_addr + reg_offs;
		reg = mbreg_find_desc(regs, n_regs, addr);

		status = mbreg_write(
			reg,
			addr,
			n_req_regs-reg_offs,
			req_write_data + reg_offs*2u,
			&n_regs_written);
		if (status!=MB_OK) return status;
		if (n_regs_written==0u) return MB_DEV_FAIL;

		if (reg->post_write_cb) {
			reg->post_write_cb();
		}

		/* Advance by the actual written register size to handle
		   sub-registers correctly */
		reg_offs += (uint16_t)n_regs_written;
	}

	if (inst->commit_regs_write_cb) {
		inst->commit_regs_write_cb(inst);
	}

	/* Prepare response */
	if (res) {
		u16tobe(start_addr, res->p+1u);
		u16tobe(n_req_regs, res->p+3u);
		res->size = 5u;
	}

	return MB_OK;
}

extern enum mbstatus_e mbfn_read_regs(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	uint16_t start_addr, n_req_regs;

	if (!inst || !regs || !req || !res) return MB_DEV_FAIL;

	if ((req[0]!=MBFC_READ_HOLDING_REGS) && (req[0]!=MBFC_READ_INPUT_REGS)) {
		return MB_DEV_FAIL;
	}

	if (req_len != 5u) { /* Required request data is one word for addr and one for n regs */
		return MB_ILLEGAL_DATA_VAL;
	}

	start_addr = betou16(req+1u);
	n_req_regs = betou16(req+3u); /* Amount of 16 bit registers to read */

	return read_regs(
		inst,
		regs,
		n_regs,
		start_addr,
		n_req_regs,
		res,
		req[0] == MBFC_READ_HOLDING_REGS);
}

extern enum mbstatus_e mbfn_write_reg(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	const struct mbreg_desc_s *reg;
	enum mbstatus_e status;
	size_t n_written;
	uint16_t addr;

	if (!inst || !regs || !req || !res) return MB_DEV_FAIL;

	if (req[0]!=MBFC_WRITE_SINGLE_REG) {
		return MB_DEV_FAIL;
	}

	if (req_len != 5u) { /* One word address and one word data */
		return MB_ILLEGAL_DATA_VAL;
	}

	addr = betou16(req+1u);

	if (!(reg = mbreg_find_desc(regs, n_regs, addr))) {
		return MB_ILLEGAL_DATA_ADDR;
	}

	if (mbreg_write_allowed(reg, addr, addr, 1u, req+3u) != 1u) {
		return MB_ILLEGAL_DATA_ADDR;
	}

	status = mbreg_write(reg, addr, 1u, req+3u, &n_written);
	if (status!=MB_OK) return status;
	if (n_written!=1u) return MB_DEV_FAIL;

	if (reg->post_write_cb) {
		reg->post_write_cb();
	}
	if (inst->commit_regs_write_cb) {
		inst->commit_regs_write_cb(inst);
	}

	/* Prepare response */
	res->p[1] = req[1];
	res->p[2] = req[2];
	res->p[3] = req[3];
	res->p[4] = req[4];
	res->size = 5u;

	return MB_OK;
}

extern enum mbstatus_e mbfn_write_regs(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	uint16_t start_addr, n_req_regs;
	uint8_t byte_count;

	if (!inst || !regs || !req || !res) return MB_DEV_FAIL;

	if (req[0]!=MBFC_WRITE_MULTIPLE_REGS) {
		return MB_DEV_FAIL;
	}

	/* Check that request data is at least big enough for fields 'addr',
	   'nr of regs' and 'byte count' (u16 + u16 + u8): */
	if (req_len < 6u) {
		return MB_ILLEGAL_DATA_VAL;
	}

	start_addr = betou16(req+1u);
	n_req_regs = betou16(req+3u); /* Amount of 16 bit registers to write */
	byte_count = req[5];

	if ((req_len-6u) != byte_count) {
		return MB_ILLEGAL_DATA_VAL;
	}

	return write_regs(
		inst,
		regs,
		n_regs,
		start_addr,
		n_req_regs,
		byte_count,
		req+6u,
		res);
}

extern enum mbstatus_e mbfn_read_write_regs(
	const struct mbinst_s *inst,
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	enum mbstatus_e status;
	uint16_t read_start_addr, n_read_regs;
	uint16_t write_start_addr, n_write_regs;
	uint8_t write_byte_count;

	if (!inst || !regs || !req || !res) return MB_DEV_FAIL;

	if (req[0]!=MBFC_READ_WRITE_REGS) {
		return MB_DEV_FAIL;
	}

	/* Check that request data is at least big enough for 'read start addr', 'read n regs', 'write addr',
	   'write n regs' and 'write byte count' */
	if (req_len < 10u) {
		return MB_ILLEGAL_DATA_VAL;
	}

	read_start_addr = betou16(req+1u);
	n_read_regs = betou16(req+3u);

	write_start_addr = betou16(req+5u);
	n_write_regs = betou16(req+7u);
	write_byte_count = req[9];

	/* Perform a dry run read to ensure this read is valid (No locks etc.) */
	status = read_regs(
		inst,
		regs,
		n_regs,
		read_start_addr,
		n_read_regs,
		NULL, /* Dry run */
		1); /* is_hold_reg = 1 since function 0x17 only operates on holding registers */
	if (status != MB_OK) {
		return status;
	}

	/* Perform write operation first (as per Modbus spec) */
	status = write_regs(
		inst,
		regs,
		n_regs,
		write_start_addr,
		n_write_regs,
		write_byte_count,
		req+10u,
		NULL); /* No response needed for write part */
	if (status != MB_OK) {
		return status;
	}

	/* Then perform read operation (this generates the actual response) */
	return read_regs(
		inst,
		regs,
		n_regs,
		read_start_addr,
		n_read_regs,
		res,
		1); /* is_hold_reg = 1 since function 0x17 only operates on holding registers */
}
