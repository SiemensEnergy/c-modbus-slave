/**
 * @file mbreg.c
 * @brief Implementation of Modbus register handling functions
 * @author Jonas Almås
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

#include "mbreg.h"
#include "endian.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {BSEARCH_THRESHOLD=16};

static size_t min(size_t a, size_t b)
{
	return a < b ? a : b;
}

/**
 * @brief Swaps the word order of a 32-bit value in-place
 * @param data Pointer to 4 bytes representing the value to swap
 */
static void swap_words_u32(uint8_t *data)
{
	uint8_t tmp;

	tmp = data[0];
	data[0] = data[2];
	data[2] = tmp;

	tmp = data[1];
	data[1] = data[3];
	data[3] = tmp;
}

/**
 * @brief Swaps the word order of a 64-bit value in-place
 *
 * @param data Pointer to 8 bytes representing the value to swap
 */
static void swap_words_u64(uint8_t *data)
{
	uint8_t tmp;

	tmp = data[0];
	data[0] = data[6];
	data[6] = tmp;

	tmp = data[1];
	data[1] = data[7];
	data[7] = tmp;

	tmp = data[2];
	data[2] = data[4];
	data[4] = tmp;

	tmp = data[3];
	data[3] = data[5];
	data[5] = tmp;
}

extern size_t mbreg_size(const struct mbreg_desc_s *reg)
{
	size_t sz = (reg->type & MRTYPE_SIZE_MASK) / 8;
	return sz==1 ? 2 : sz; /* 8-bit regs don't exist in Modbus */
}

/**
 * Check if address and descriptor matches
 */
static int is_addr_desc_match(const struct mbreg_desc_s *reg, uint16_t addr)
{
	size_t reg_size_w;

	if (addr == reg->address) {
		return 1;
	} else if (addr > reg->address) {
		reg_size_w = mbreg_size(reg) / 2;
		if (reg->type & MRTYPE_BLOCK) {
			if (addr < reg->address + reg->n_block_entries*reg_size_w) {
				return 1;
			}
		} else if (addr < (reg->address + reg_size_w)) {
			return 1;
		}
	}

	return 0;
}

extern const struct mbreg_desc_s *mbreg_find_desc(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t addr)
{
	const struct mbreg_desc_s *reg;
	size_t l, m, r;
	size_t i;

	if (!regs || n_regs==0) return NULL;

	if (n_regs > BSEARCH_THRESHOLD) { /* Only use binary search for larger descriptor sets */
		l = 0;
		r = n_regs - 1;

		while (l <= r) {
			m = l + (r - l) / 2;
			reg = regs + m;

			if (is_addr_desc_match(reg, addr)) {
				return reg;
			} else if (reg->address < addr) {
				l = m + 1;
			} else {
				if (m == 0) break; /* Prevent underflow */
				r = m - 1;
			}
		}
	} else {
		for (i=0; i<n_regs; ++i) {
			reg = regs + i;
			if (is_addr_desc_match(reg, addr)) {
				return reg;
			}
		}
	}

	return NULL;
}

/**
 * @brief Check if register is configure correctly for reading from pointer
 */
static int read_ptr_ok(const struct mbreg_desc_s *reg)
{
	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: return !!reg->read.pu8;
	case MRTYPE_U16: return !!reg->read.pu16;
	case MRTYPE_U32: return !!reg->read.pu32;
	case MRTYPE_U64: return !!reg->read.pu64;
	case MRTYPE_I8: return !!reg->read.pi8;
	case MRTYPE_I16: return !!reg->read.pi16;
	case MRTYPE_I32: return !!reg->read.pi32;
	case MRTYPE_I64: return !!reg->read.pi64;
	case MRTYPE_F32: return !!reg->read.pf32;
	case MRTYPE_F64: return !!reg->read.pf64;
	default: return 0;
	}
}

/**
 * @brief Check if register is configure correctly for reading from function
 */
static int read_fn_ok(const struct mbreg_desc_s *reg)
{
	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: return !!reg->read.fu8;
	case MRTYPE_U16: return !!reg->read.fu16;
	case MRTYPE_U32: return !!reg->read.fu32;
	case MRTYPE_U64: return !!reg->read.fu64;
	case MRTYPE_I8: return !!reg->read.fi8;
	case MRTYPE_I16: return !!reg->read.fi16;
	case MRTYPE_I32: return !!reg->read.fi32;
	case MRTYPE_I64: return !!reg->read.fi64;
	case MRTYPE_F32: return !!reg->read.ff32;
	case MRTYPE_F64: return !!reg->read.ff64;
	default: return 0;
	}
}

/**
 * @brief Check if register is configure correctly for writing to pointer
 */
static int write_ptr_ok(const struct mbreg_desc_s *reg)
{
	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: return !!reg->write.pu8;
	case MRTYPE_U16: return !!reg->write.pu16;
	case MRTYPE_U32: return !!reg->write.pu32;
	case MRTYPE_U64: return !!reg->write.pu64;
	case MRTYPE_I8: return !!reg->write.pi8;
	case MRTYPE_I16: return !!reg->write.pi16;
	case MRTYPE_I32: return !!reg->write.pi32;
	case MRTYPE_I64: return !!reg->write.pi64;
	case MRTYPE_F32: return !!reg->write.pf32;
	case MRTYPE_F64: return !!reg->write.pf64;
	default: return 0;
	}
}

/**
 * @brief Check if register is configure correctly for writing to function
 */
static int write_fn_ok(const struct mbreg_desc_s *reg)
{
	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: return !!reg->write.fu8;
	case MRTYPE_U16: return !!reg->write.fu16;
	case MRTYPE_U32: return !!reg->write.fu32;
	case MRTYPE_U64: return !!reg->write.fu64;
	case MRTYPE_I8: return !!reg->write.fi8;
	case MRTYPE_I16: return !!reg->write.fi16;
	case MRTYPE_I32: return !!reg->write.fi32;
	case MRTYPE_I64: return !!reg->write.fi64;
	case MRTYPE_F32: return !!reg->write.ff32;
	case MRTYPE_F64: return !!reg->write.ff64;
	default: return 0;
	}
}

/**
 * @return n 16-bit registers read
 */
static int read_val(const struct mbreg_desc_s *reg, uint8_t *res)
{
	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: u16tobe((uint16_t)reg->read.u8, res); break;
	case MRTYPE_U16: u16tobe(reg->read.u16, res); break;
	case MRTYPE_U32: u32tobe(reg->read.u32, res); break;
	case MRTYPE_U64: u64tobe(reg->read.u64, res); break;
	case MRTYPE_I8: i16tobe((int16_t)reg->read.i8, res); break;
	case MRTYPE_I16: i16tobe(reg->read.i16, res); break;
	case MRTYPE_I32: i32tobe(reg->read.i32, res); break;
	case MRTYPE_I64: i64tobe(reg->read.i64, res); break;
	case MRTYPE_F32: f32tobe(reg->read.f32, res); break;
	case MRTYPE_F64: f64tobe(reg->read.f64, res); break;
	default: return 0;
	}

	return 1;
}

/**
 * @return n 16-bit registers read
 */
static int read_ptr(const struct mbreg_desc_s *reg, uint8_t *res)
{
	if (!read_ptr_ok(reg)) return 0;

	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: u16tobe((uint16_t)*reg->read.pu8, res); break;
	case MRTYPE_U16: u16tobe(*reg->read.pu16, res); break;
	case MRTYPE_U32: u32tobe(*reg->read.pu32, res); break;
	case MRTYPE_U64: u64tobe(*reg->read.pu64, res); break;
	case MRTYPE_I8: i16tobe((int16_t)*reg->read.pi8, res); break;
	case MRTYPE_I16: i16tobe(*reg->read.pi16, res); break;
	case MRTYPE_I32: i32tobe(*reg->read.pi32, res); break;
	case MRTYPE_I64: i64tobe(*reg->read.pi64, res); break;
	case MRTYPE_F32: f32tobe(*reg->read.pf32, res); break;
	case MRTYPE_F64: f64tobe(*reg->read.pf64, res); break;
	default: return 0;
	}

	return 1;
}

/**
 * @return n 16-bit registers read
 */
static int read_block(const struct mbreg_desc_s *reg, uint16_t addr, uint8_t *res)
{
	size_t reg_size_w, ix;
	uint16_t start_addr;

	if (!read_ptr_ok(reg)) return 0;

	reg_size_w = mbreg_size(reg) / 2;
	ix = (addr - reg->address) / reg_size_w;
	start_addr = reg->address + ix * reg_size_w;

	if ((size_t)(addr-start_addr) >= reg_size_w) {
		return 0;
	}

	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: u16tobe((uint16_t)*(reg->read.pu8 + ix), res); break;
	case MRTYPE_U16: u16tobe(*(reg->read.pu16 + ix), res); break;
	case MRTYPE_U32: u32tobe(*(reg->read.pu32 + ix), res); break;
	case MRTYPE_U64: u64tobe(*(reg->read.pu64 + ix), res); break;
	case MRTYPE_I8: i16tobe((int16_t)*(reg->read.pi8 + ix), res); break;
	case MRTYPE_I16: i16tobe(*(reg->read.pi16 + ix), res); break;
	case MRTYPE_I32: i32tobe(*(reg->read.pi32 + ix), res); break;
	case MRTYPE_I64: i64tobe(*(reg->read.pi64 + ix), res); break;
	case MRTYPE_F32: f32tobe(*(reg->read.pf32 + ix), res); break;
	case MRTYPE_F64: f64tobe(*(reg->read.pf64 + ix), res); break;
	default: return 0;
	}

	return 1;
}

/**
 * @return n 16-bit registers read
 */
static int read_fn(const struct mbreg_desc_s *reg, uint8_t *res)
{
	if (!read_fn_ok(reg)) return 0;

	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: u16tobe((uint16_t)reg->read.fu8(), res); break;
	case MRTYPE_U16: u16tobe(reg->read.fu16(), res); break;
	case MRTYPE_U32: u32tobe(reg->read.fu32(), res); break;
	case MRTYPE_U64: u64tobe(reg->read.fu64(), res); break;
	case MRTYPE_I8: i16tobe((int16_t)reg->read.fi8(), res); break;
	case MRTYPE_I16: i16tobe(reg->read.fi16(), res); break;
	case MRTYPE_I32: i32tobe(reg->read.fi32(), res); break;
	case MRTYPE_I64: i64tobe(reg->read.fi64(), res); break;
	case MRTYPE_F32: f32tobe(reg->read.ff32(), res); break;
	case MRTYPE_F64: f64tobe(reg->read.ff64(), res); break;
	default: return 0;
	}

	return 1;
}

/**
 * @retval Number of 16-bit words actually read
 * @retval MBREG_READ_DEV_FAIL (SIZE_MAX) Device fault
 */
static size_t read_partial(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	size_t n_remaining_regs,
	uint8_t *res,
	int swap_words)
{
	uint8_t buf[MRTYPE_SIZE_MAX/8];
	size_t reg_size, buf_offset, n_copy;
	int ok;

	reg_size = mbreg_size(reg);

	if (reg->type & MRTYPE_BLOCK) {
		ok=read_block(reg, addr, buf);
	} else {
		if (((addr - reg->address)*2u) >= reg_size) {
			return MBREG_READ_DEV_FAIL;
		}

		switch (reg->access & MRACC_R_MASK) {
		case MRACC_R_VAL: ok=read_val(reg, buf); break;
		case MRACC_R_PTR: ok=read_ptr(reg, buf); break;
		case MRACC_R_FN: ok=read_fn(reg, buf); break;
		default: return MBREG_READ_DEV_FAIL;
		}
	}

	if (!ok) return MBREG_READ_DEV_FAIL;

	if (swap_words) {
		switch (reg_size) {
		case 4u: swap_words_u32(buf); break;
		case 8u: swap_words_u64(buf); break;
		default: break;
		}
	}

	buf_offset = (addr - reg->address) * 2;
	n_copy = min(reg_size-buf_offset, n_remaining_regs*2);

	if (res) {
		memcpy(res, buf+buf_offset, n_copy);
	}

	return n_copy / 2;
}

static int read_full(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	uint8_t *res,
	int swap_words)
{
	int ok;

	if (reg->type & MRTYPE_BLOCK) {
		ok=read_block(reg, addr, res);
	} else {
		switch (reg->access & MRACC_R_MASK) {
		case MRACC_R_VAL: ok=read_val(reg, res); break;
		case MRACC_R_PTR: ok=read_ptr(reg, res); break;
		case MRACC_R_FN: ok=read_fn(reg, res); break;
		default: ok=0; break;
		}
	}

	if (!ok) return 0;

	if (swap_words) {
		switch (mbreg_size(reg)) {
		case 4u: swap_words_u32(res); break;
		case 8u: swap_words_u64(res); break;
		default: break;
		}
	}

	return ok;
}

extern size_t mbreg_read(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	size_t n_remaining_regs,
	uint8_t *res,
	int swap_words)
{
	size_t reg_size_w;

	if (!reg) return MBREG_READ_DEV_FAIL;
	if (n_remaining_regs == 0u) return MBREG_READ_DEV_FAIL;
	if (addr < reg->address) return MBREG_READ_DEV_FAIL;

	if (!(reg->access & MRACC_R_MASK)) return MBREG_READ_NO_ACCESS; /* Check if read is allowed */
	if (reg->rlock_cb && reg->rlock_cb()) return MBREG_READ_LOCKED; /* Check if read locked */

	reg_size_w = mbreg_size(reg) / 2;
	if (reg_size_w == 0u) return MBREG_READ_DEV_FAIL;

	if (n_remaining_regs < reg_size_w || (addr - reg->address) % reg_size_w) {
		return read_partial(reg, addr, n_remaining_regs, res, swap_words);
	} else {
		if (res) { /* Not dry run */
			if (!read_full(reg, addr, res, swap_words)) return MBREG_READ_DEV_FAIL;
		}
		return reg_size_w;
	}
}

extern int mbreg_write_allowed(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	uint16_t start_addr,
	size_t n_remaining_regs,
	const uint8_t *val)
{
	size_t reg_size_w, offset, n_write_bytes;

	if (!reg || !val) return 0u;

	if (n_remaining_regs == 0u) return 0u;
	if (addr < reg->address) return 0u;

	/* Check if write is allowed */
	if (!(reg->access & MRACC_W_MASK)) return 0u;

	/* Check if write is locked */
	if (reg->wlock_cb && reg->wlock_cb()) {
		if (!reg->wlock_override_cb
				|| !reg->wlock_override_cb(
					reg,
					start_addr,
					n_remaining_regs,
					val)) {
			return 0u;
		}
	}

	reg_size_w = mbreg_size(reg) / 2;
	if (reg_size_w == 0u) return 0u;

	/* Make sure we have enough data for this register */
	if (n_remaining_regs < reg_size_w) {
		return 0u;
	}

	/* Return n registers that will be written to */
	if (n_remaining_regs < reg_size_w || (addr - reg->address) % reg_size_w) { /* Partial reg write */
		offset = (addr - reg->address) * 2;
		n_write_bytes = min(reg_size_w*2 - offset, n_remaining_regs*2);
		return n_write_bytes / 2;
	} else { /* Full register */
		return reg_size_w;
	}
}

/**
 * @return n 16-bit registers written
 */
static enum mbstatus_e write_ptr(
	const struct mbreg_desc_s *reg,
	const uint8_t *val)
{
	if (!write_ptr_ok(reg)) return MB_DEV_FAIL;

	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: *reg->write.pu8 = (uint8_t)betou16(val); break;
	case MRTYPE_U16: *reg->write.pu16 = betou16(val); break;
	case MRTYPE_U32: *reg->write.pu32 = betou32(val); break;
	case MRTYPE_U64: *reg->write.pu64 = betou64(val); break;
	case MRTYPE_I8: *reg->write.pi8 = (int8_t)betoi16(val); break;
	case MRTYPE_I16: *reg->write.pi16 = betoi16(val); break;
	case MRTYPE_I32: *reg->write.pi32 = betoi32(val); break;
	case MRTYPE_I64: *reg->write.pi64 = betoi64(val); break;
	case MRTYPE_F32: *reg->write.pf32 = betof32(val); break;
	case MRTYPE_F64: *reg->write.pf64 = betof64(val); break;
	default: return MB_DEV_FAIL;
	}

	return MB_OK;
}

/**
 * @return n 16-bit registers written
 */
static enum mbstatus_e write_ptr_partial(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	size_t n_remaining_regs,
	const uint8_t *val,
	size_t *n_written)
{
	uint8_t buf[MRTYPE_SIZE_MAX/8];
	size_t reg_size, buf_offset, n_copy;

	if (!write_ptr_ok(reg)) return MB_DEV_FAIL;

	reg_size = mbreg_size(reg);

	if (((addr - reg->address)*2u) >= reg_size) {
		return MB_DEV_FAIL;
	}

	/* Read the current value into a buffer */
	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: u16tobe((uint16_t)*reg->write.pu8, buf); break;
	case MRTYPE_U16: u16tobe(*reg->write.pu16, buf); break;
	case MRTYPE_U32: u32tobe(*reg->write.pu32, buf); break;
	case MRTYPE_U64: u64tobe(*reg->write.pu64, buf); break;
	case MRTYPE_I8: i16tobe((int16_t)*reg->write.pi8, buf); break;
	case MRTYPE_I16: i16tobe(*reg->write.pi16, buf); break;
	case MRTYPE_I32: i32tobe(*reg->write.pi32, buf); break;
	case MRTYPE_I64: i64tobe(*reg->write.pi64, buf); break;
	case MRTYPE_F32: f32tobe(*reg->write.pf32, buf); break;
	case MRTYPE_F64: f64tobe(*reg->write.pf64, buf); break;
	default: return MB_DEV_FAIL;
	}

	/* Apply the partial write */
	buf_offset = (addr - reg->address) * 2;
	n_copy = min(reg_size-buf_offset, n_remaining_regs*2);
	memcpy(buf+buf_offset, val, n_copy);

	*n_written = n_copy / 2;

	/* Write the modified value */
	return write_ptr(reg, buf);
}

/**
 * @return n 16-bit registers written
 */
static enum mbstatus_e write_block(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	const uint8_t *val)
{
	size_t ix = (addr - reg->address) / (mbreg_size(reg) / 2);

	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: *(reg->write.pu8 + ix) = (uint8_t)betoi16(val); break;
	case MRTYPE_U16: *(reg->write.pu16 + ix) = betou16(val); break;
	case MRTYPE_U32: *(reg->write.pu32 + ix) = betou32(val); break;
	case MRTYPE_U64: *(reg->write.pu64 + ix) = betou64(val); break;
	case MRTYPE_I8: *(reg->write.pi8 + ix) = (int8_t)betoi16(val); break;
	case MRTYPE_I16: *(reg->write.pi16 + ix) = betoi16(val); break;
	case MRTYPE_I32: *(reg->write.pi32 + ix) = betoi32(val); break;
	case MRTYPE_I64: *(reg->write.pi64 + ix) = betoi64(val); break;
	case MRTYPE_F32: *(reg->write.pf32 + ix) = betof32(val); break;
	case MRTYPE_F64: *(reg->write.pf64 + ix) = betof64(val); break;
	default: return MB_DEV_FAIL;
	}

	return MB_OK;
}

/**
 * @return n 16-bit registers written
 */
static enum mbstatus_e write_block_partial(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	size_t n_remaining_regs,
	const uint8_t *val,
	size_t *n_written)
{
	uint8_t buf[MRTYPE_SIZE_MAX/8];
	uint16_t start_addr;
	size_t reg_size, reg_size_w, ix, buf_offset, n_copy;

	reg_size = mbreg_size(reg);
	reg_size_w = reg_size / 2;
	ix = (addr - reg->address) / reg_size_w;
	start_addr = reg->address + ix * reg_size_w;

	if (((addr - start_addr)*2u) >= reg_size) {
		return MB_DEV_FAIL;
	}

	/* Read the current value into a buffer */
	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: u16tobe((uint16_t)*(reg->write.pu8 + ix), buf); break;
	case MRTYPE_U16: u16tobe(*(reg->write.pu16 + ix), buf); break;
	case MRTYPE_U32: u32tobe(*(reg->write.pu32 + ix), buf); break;
	case MRTYPE_U64: u64tobe(*(reg->write.pu64 + ix), buf); break;
	case MRTYPE_I8: i16tobe((int16_t)*(reg->write.pi8 + ix), buf); break;
	case MRTYPE_I16: i16tobe(*(reg->write.pi16 + ix), buf); break;
	case MRTYPE_I32: i32tobe(*(reg->write.pi32 + ix), buf); break;
	case MRTYPE_I64: i64tobe(*(reg->write.pi64 + ix), buf); break;
	case MRTYPE_F32: f32tobe(*(reg->write.pf32 + ix), buf); break;
	case MRTYPE_F64: f64tobe(*(reg->write.pf64 + ix), buf); break;
	default: return MB_DEV_FAIL;
	}

	/* Apply the modification */
	buf_offset = (addr - start_addr) * 2;
	n_copy = min(reg_size-buf_offset, n_remaining_regs*2);
	memcpy(buf+buf_offset, val, n_copy);

	*n_written = n_copy / 2;

	/* Write the modified value */
	return write_block(reg, addr, buf);
}

/**
 * @return n 16-bit registers written
 */
static enum mbstatus_e write_fn(
	const struct mbreg_desc_s *reg,
	const uint8_t *val)
{
	if (!write_fn_ok(reg)) return MB_DEV_FAIL;

	switch (reg->type & MRTYPE_MASK) {
	case MRTYPE_U8: return reg->write.fu8((uint8_t)betou16(val));
	case MRTYPE_U16: return reg->write.fu16(betou16(val));
	case MRTYPE_U32: return reg->write.fu32(betou32(val));
	case MRTYPE_U64: return reg->write.fu64(betou64(val));
	case MRTYPE_I8: return reg->write.fi8((int8_t)betoi16(val));
	case MRTYPE_I16: return reg->write.fi16(betoi16(val));
	case MRTYPE_I32: return reg->write.fi32(betoi32(val));
	case MRTYPE_I64: return reg->write.fi64(betoi64(val));
	case MRTYPE_F32: return reg->write.ff32(betof32(val));
	case MRTYPE_F64: return reg->write.ff64(betof64(val));
	default: return MB_DEV_FAIL;
	}
}

extern enum mbstatus_e mbreg_write(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	size_t n_remaining_regs,
	const uint8_t *val,
	size_t *n_written)
{
	size_t reg_size_w;

	if (!reg || !val) return MB_DEV_FAIL;

	if (n_remaining_regs == 0u) return MB_DEV_FAIL;
	if (addr < reg->address) return MB_DEV_FAIL;

	reg_size_w = mbreg_size(reg) / 2;
	if (reg_size_w == 0u) return MB_DEV_FAIL;

	if (n_remaining_regs < reg_size_w || (addr - reg->address) % reg_size_w) { /* Partial reg write*/
		if (reg->type & MRTYPE_BLOCK) {
			return write_block_partial(reg, addr, n_remaining_regs, val, n_written);
		} else if ((reg->access & MRACC_W_MASK) == MRACC_W_PTR) {
			return write_ptr_partial(reg, addr, n_remaining_regs, val, n_written);
		} else {
			return MB_DEV_FAIL; /* Writing partial to a function doesn't make any sense */
		}
	} else { /* Full reg write */
		*n_written = reg_size_w;

		if (reg->type & MRTYPE_BLOCK) {
			return write_block(reg, addr, val);
		} else {
			switch (reg->access & MRACC_W_MASK) {
			case MRACC_W_PTR: return write_ptr(reg, val);
			case MRACC_W_FN: return write_fn(reg, val);
			default: return MB_DEV_FAIL;
			}
		}
	}
}
