/**
 * @file mbfn_coils.c
 * @brief Implementation of Modbus coil function handlers
 * @author Jonas Almås
 *
 * MISRA Deviations:
 * - Rule 12.1: The precedence of operators within expressions should be made explicit
 * - Rule 12.3: The comma operator should not be used
 * - Rule 13.4: The result of an assignment operator should not be used
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

#include "mbfn_coils.h"
#include "endian.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
	MBCOIL_N_READ_MAX=0x07D0u,
	MBCOIL_N_WRITE_MAX=0x07B0u,
};

extern enum mbstatus_e mbfn_read_coils(
	const struct mbinst_s *inst,
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	uint16_t start_addr, quantity, addr;
	uint8_t byte_count;
	size_t i;
	const struct mbcoil_desc_s *coil;

	if (!inst || !coils || !req || !res) {
		return MB_DEV_FAIL;
	}

	if ((req[0]!=MBFC_READ_COILS) && (req[0]!=MBFC_READ_DISC_INPUTS)) {
		return MB_DEV_FAIL;
	}

	if (req_len != 5u) {
		return MB_ILLEGAL_DATA_VAL;
	}

	start_addr = betou16(req+1u);
	quantity = betou16(req+3u);

	if ((quantity==0u) || (quantity>MBCOIL_N_READ_MAX)) { /* Validate quantity */
		return MB_ILLEGAL_DATA_VAL;
	}

	/* If we read multiple coils and one of them doesn't exist,
	   we just padd that wil zeros.
	   We don't want to do this if the first coils is missing.
	 */
	if (!mbcoil_find_desc(coils, n_coils, start_addr)) {
		return MB_ILLEGAL_DATA_ADDR;
	}

	byte_count = (uint8_t)((quantity + 7u) / 8u);
	res->p[1] = byte_count;
	res->size = 2u + (size_t)byte_count;

	(void)memset(res->p+2u, 0, byte_count); /* Clear all response bytes */

	/* Read coils */
	for (i=0u; i<quantity; ++i) {
		addr = start_addr + (uint16_t)i;
		if ((coil = mbcoil_find_desc(coils, n_coils, addr))) {
			switch (mbcoil_read(coil)) {
			case MBCOIL_READ_OFF: break;
			case MBCOIL_READ_ON:
				res->p[2u + i/8u] |= (uint8_t)(1u<<(i%8u));
				break;
			case MBCOIL_READ_LOCKED: return MB_ILLEGAL_DATA_ADDR;
			case MBCOIL_READ_NO_ACCESS: break; /* Leave coils without read access as 0 */
			case MBCOIL_READ_DEV_FAIL: return MB_DEV_FAIL;
			default: return MB_DEV_FAIL;
			}
		}
		/* If coil doesn't exist, it's left as 0 (already cleared above) */
	}

	return MB_OK;
}

extern enum mbstatus_e mbfn_write_coil(
	const struct mbinst_s *inst,
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	uint16_t coil_addr, coil_value;
	enum mbstatus_e status;
	const struct mbcoil_desc_s *coil;

	if (!inst || !coils || !req || !res) {
		return MB_DEV_FAIL;
	}

	if (req[0]!=MBFC_WRITE_SINGLE_COIL) {
		return MB_DEV_FAIL;
	}

	if (req_len != 5u) {
		return MB_ILLEGAL_DATA_VAL;
	}

	coil_addr = betou16(req+1u);
	coil_value = betou16(req+3u);

	/* Validate coil value (must be 0x0000 or 0xFF00) */
	if ((coil_value != MBCOIL_OFF) && (coil_value != MBCOIL_ON)) {
		return MB_ILLEGAL_DATA_VAL;
	}

	coil = mbcoil_find_desc(coils, n_coils, coil_addr);
	if (!coil) {
		return MB_ILLEGAL_DATA_ADDR;
	}

	if (!mbcoil_write_allowed(coil)) {
		return MB_ILLEGAL_DATA_ADDR;
	}

	status = mbcoil_write(coil, !!coil_value);
	if (status!=MB_OK) {
		return status;
	}

	if (coil->post_write_cb) {
		coil->post_write_cb();
	}
	if (inst->commit_coils_write_cb) {
		inst->commit_coils_write_cb(inst);
	}

	/* Prepare response (echo the request) */
	res->p[1] = req[1];
	res->p[2] = req[2];
	res->p[3] = req[3];
	res->p[4] = req[4];
	res->size = 5u;

	return MB_OK;
}

extern enum mbstatus_e mbfn_write_coils(
	const struct mbinst_s *inst,
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	const uint8_t *req,
	size_t req_len,
	struct mbpdu_buf_s *res)
{
	uint16_t start_addr, quantity, addr;
	uint8_t byte_count;
	size_t i;
	enum mbstatus_e status;
	const struct mbcoil_desc_s *coil;

	if (!inst || !coils || !req || !res) {
		return MB_DEV_FAIL;
	}

	if (req[0]!=MBFC_WRITE_MULTIPLE_COILS) {
		return MB_DEV_FAIL;
	}

	/* Check that request data is at least big enough for fields
	  'fc', 'addr', 'nr of coils', 'byte count' and 'data'... (u8 + u16 + u16 + u8 + ...) */
	if (req_len < 7u) { /* Check minimum request length */
		return MB_ILLEGAL_DATA_VAL;
	}

	start_addr = betou16(req+1u);
	quantity = betou16(req+3u);
	byte_count = req[5];

	if ((quantity==0u) || (quantity>MBCOIL_N_WRITE_MAX)) { /* Validate quantity */
		return MB_ILLEGAL_DATA_VAL;
	}

	if (byte_count != ((quantity+7u)/8u)) { /* Validate byte count */
		return MB_ILLEGAL_DATA_VAL;
	}

	if (req_len != (6u+byte_count)) { /* Check request length */
		return MB_ILLEGAL_DATA_VAL;
	}

	/* Ensure all coils exists and can be written to before writing anything */
	for (i=0u; i<quantity; ++i) {
		addr = start_addr + (uint16_t)i;
		if (!(coil = mbcoil_find_desc(coils, n_coils, addr))) {
			return MB_ILLEGAL_DATA_ADDR;
		}

		if (!mbcoil_write_allowed(coil)) {
			return MB_ILLEGAL_DATA_ADDR;
		}
	}

	/* Write coils */
	for (i=0u; i<quantity; ++i) {
		addr = start_addr + (uint16_t)i;
		coil = mbcoil_find_desc(coils, n_coils, addr);

		status = mbcoil_write(coil, !!(req[6u + i/8u] & (uint8_t)(1u<<(i%8u))));
		if (status!=MB_OK) {
			return status;
		}

		if (coil->post_write_cb) {
			coil->post_write_cb();
		}
	}

	/* Call commit callback if it exists */
	if (inst->commit_coils_write_cb) {
		inst->commit_coils_write_cb(inst);
	}

	/* Prepare response */
	u16tobe(start_addr, res->p+1u);
	u16tobe(quantity, res->p+3u);
	res->size = 5u;

	return MB_OK;
}
