/**
 * @file mbcoil.c
 * @brief Implementation of Modbus coil handling functions
 * @author Jonas Almås
 *
 * MISRA Deviations:
 * - Rule 10.1: Operands shall not be of an inappropriate essential type
 * - Rule 12.3: The comma operator should not be used
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

#include "mbcoil.h"
#include <stddef.h>

enum {BSEARCH_THRESHOLD=16u};

extern const struct mbcoil_desc_s *mbcoil_find_desc(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t addr)
{
	const struct mbcoil_desc_s *coil;
	size_t l, m, r;
	size_t i;

	if (!coils || (n_coils==0u)) return NULL;

	if (n_coils > BSEARCH_THRESHOLD) { /* Only use binary search for larger descriptor sets */
		l = 0u;
		r = n_coils - 1u;

		while (l <= r) {
			m = l + (r - l) / 2u;
			coil = coils + m;

			if (coil->address < addr) {
				l = m + 1u;
			} else if (coil->address > addr) {
				if (m == 0u) break; /* Prevent underflow */
				r = m - 1u;
			} else {
				return coil;
			}
		}
	} else {
		for (i=0u; i<n_coils; ++i) {
			coil = coils + i;
			if (coil->address == addr) {
				return coil;
			}
		}
	}

	return NULL;
}

extern int mbcoil_read(const struct mbcoil_desc_s *coil)
{
	if (!coil) return MBCOIL_READ_DEV_FAIL;

	/* Check read permissions */
	if (coil->rlock_cb && coil->rlock_cb()) {
		return MBCOIL_READ_LOCKED;
	}

	switch (coil->access & MCACC_R_MASK) {
	case MCACC_R_VAL:
		return !!coil->read.val;
	case MCACC_R_PTR:
		return (coil->read.ptr && (coil->read.ix<8u))
			? (!!(*coil->read.ptr & (1u<<coil->read.ix)))
			: MBCOIL_READ_DEV_FAIL;
	case MCACC_R_FN:
		return coil->read.fn
			? !!coil->read.fn()
			: MBCOIL_READ_DEV_FAIL;
	default:
		return MBCOIL_READ_NO_ACCESS;
	}
}

extern int mbcoil_write_allowed(const struct mbcoil_desc_s *coil)
{
	if (!coil) return 0;

	/* Check write permissions */
	if (coil->wlock_cb && coil->wlock_cb()) {
		return 0;
	}

	return 1;
}

extern enum mbstatus_e mbcoil_write(const struct mbcoil_desc_s *coil, uint8_t value)
{
	if (!coil) return MB_DEV_FAIL;

	switch (coil->access & MCACC_W_MASK) {
	case MCACC_W_PTR:
		if (!coil->write.ptr || (coil->write.ix>7u)) return MB_DEV_FAIL;
		if (value) {
			*coil->write.ptr |= (uint8_t)(1u<<coil->write.ix);
		} else {
			*coil->write.ptr &= (uint8_t)(~(1u<<coil->write.ix));
		}
		return MB_OK;
	case MCACC_W_FN:
		if (!coil->write.fn) return MB_DEV_FAIL;
		return coil->write.fn(!!value);
	default:
		return MB_DEV_FAIL;
	}
}
