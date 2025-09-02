/**
 * @file mbcoil.c
 * @brief Implementation of Modbus coil handling functions
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

#include "mbcoil.h"
#include <stddef.h>

enum {BSEARCH_THRESHOLD=16};

extern const struct mbcoil_desc_s *mbcoil_find_desc(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t addr)
{
	const struct mbcoil_desc_s *coil;
	size_t l, m, r;
	size_t i;

	if (!coils || n_coils==0) return NULL;

	if (n_coils > BSEARCH_THRESHOLD) { /* Only use binary search for larger descriptor sets */
		l = 0;
		r = n_coils - 1;

		while (l <= r) {
			m = l + (r - l) / 2;
			coil = coils + m;

			if (coil->address < addr) {
				l = m + 1;
			} else if (coil->address > addr) {
				if (m == 0) break; /* Prevent underflow */
				r = m - 1;
			} else {
				return coil;
			}
		}
	} else {
		for (i=0; i<n_coils; ++i) {
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
	if (!coil) return -1;

	/* Check read permissions */
	if (coil->rlock_cb && coil->rlock_cb()) {
		return -1;
	}

	switch (coil->access & MCACC_R_MASK) {
	case MCACC_R_VAL:
		return !!coil->read.val;
	case MCACC_R_PTR:
		return (coil->read.ptr && coil->read.ix<8)
			? !!((*coil->read.ptr) & (1<<coil->read.ix))
			: -1;
	case MCACC_R_FN:
		return coil->read.fn
			? !!coil->read.fn()
			: -1;
	default:
		return -1;
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
		if (!coil->write.ptr || coil->write.ix>7) return MB_DEV_FAIL;
		if (value) {
			*coil->write.ptr |= 1<<coil->write.ix;
		} else {
			*coil->write.ptr &= ~(1<<coil->write.ix);
		}
		return MB_OK;
	case MCACC_W_FN:
		if (!coil->write.fn) return MB_DEV_FAIL;
		return coil->write.fn(!!value);
	default:
		return MB_DEV_FAIL;
	}
}
