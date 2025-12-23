/**
 * @file mbtest.c
 * @brief Implementation of Modbus test and validation functions
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

#include "mbtest.h"
#include "mbcoil.h"
#include "mbreg.h"

extern int mbtest_coils_asc(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr)
{
	uint16_t prev_addr;
	size_t i;

	if (!coils || !n_coils) return 1;

	prev_addr = coils[0].address;

	for (i=1; i<n_coils; ++i) {
		if (coils[i].address <= prev_addr) {
			if (issue_addr) *issue_addr = coils[i].address;
			return 0;
		}
		prev_addr = coils[i].address;
	}

	return 1;
}

extern int mbtest_coils_valid_access(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr)
{
	const struct mbcoil_desc_s *coil;
	size_t i;

	if (!coils || !n_coils) return 1;

	for (i=0; i<n_coils; ++i) {
		coil = coils+i;

		/* Don't allow coils without any access */
		if (coil->access == 0) {
			if (issue_addr) *issue_addr = coil->address;
			return 0;
		}

		/* Check read access */
		if (coil->access & MCACC_R_MASK) {
			switch (coil->access & MCACC_R_MASK) {
			case MCACC_R_VAL:
				/* Value access is always valid */
				break;
			case MCACC_R_PTR:
				if (coil->read.ptr == NULL) {
					if (issue_addr) *issue_addr = coil->address;
					return 0;
				}
				break;
			case MCACC_R_FN:
				if (coil->read.fn == NULL) {
					if (issue_addr) *issue_addr = coil->address;
					return 0;
				}
				break;
			default:
				/* Multiple read access methods specified */
				if (issue_addr) *issue_addr = coil->address;
				return 0;
			}
		}

		/* Check write access */
		if (coil->access & MCACC_W_MASK) {
			switch (coil->access & MCACC_W_MASK) {
			case MCACC_W_PTR:
				if (coil->write.ptr == NULL) {
					if (issue_addr) *issue_addr = coil->address;
					return 0;
				}
				break;
			case MCACC_W_FN:
				if (coil->write.fn == NULL) {
					if (issue_addr) *issue_addr = coil->address;
					return 0;
				}
				break;
			default:
				/* Multiple write access methods specified */
				if (issue_addr) *issue_addr = coil->address;
				return 0;
			}
		}
	}

	return 1;
}

extern int mbtest_coils_valid_bit_index(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr)
{
	const struct mbcoil_desc_s *coil;
	size_t i;

	if (!coils || !n_coils) return 1;

	for (i=0; i<n_coils; ++i) {
		coil = coils+i;

		/* Check read bit index if using pointer access */
		if ((coil->access & MCACC_R_MASK) == MCACC_R_PTR) {
			if (coil->read.ix > 7) {
				if (issue_addr) *issue_addr = coil->address;
				return 0;
			}
		}

		/* Check write bit index if using pointer access */
		if ((coil->access & MCACC_W_MASK) == MCACC_W_PTR) {
			if (coil->write.ix > 7) {
				if (issue_addr) *issue_addr = coil->address;
				return 0;
			}
		}
	}

	return 1;
}

extern int mbtest_coils_no_duplicates(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr)
{
	size_t i, j;

	if (!coils || !n_coils) return 1;

	for (i=0; i<n_coils; ++i) {
		for (j=i+1; j<n_coils; ++j) {
			if (coils[i].address == coils[j].address) {
				if (issue_addr) *issue_addr = coils[i].address;
				return 0;
			}
		}
	}

	return 1;
}

extern int mbtest_coils_validate_all(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t *issue_addr)
{
	/* Run all coil validation tests */
	if (!mbtest_coils_asc(coils, n_coils, issue_addr)) return 0;
	if (!mbtest_coils_valid_access(coils, n_coils, issue_addr)) return 0;
	if (!mbtest_coils_valid_bit_index(coils, n_coils, issue_addr)) return 0;
	if (!mbtest_coils_no_duplicates(coils, n_coils, issue_addr)) return 0;

	return 1;
}

extern int mbtest_regs_asc(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr)
{
	size_t i;
	uint16_t prev_addr;

	if (!regs || !n_regs) return 1;

	prev_addr = regs[0].address;

	for (i=1; i<n_regs; ++i) {
		if (regs[i].address <= prev_addr) {
			if (issue_addr) *issue_addr = regs[i].address;
			return 0;
		}
		prev_addr = regs[i].address;
	}

	return 1;
}

extern int mbtest_regs_valid_size(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr)
{
	size_t i;

	if (!regs || !n_regs) return 1;

	for (i=0; i<n_regs; ++i) {
		if (mbreg_size(regs+i) == 0) {
			if (issue_addr) *issue_addr = regs[i].address;
			return 0;
		}
	}

	return 1;
}

extern int mbtest_regs_dont_overlap(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr)
{
	const struct mbreg_desc_s *reg;
	uint16_t prev_addr;
	size_t i, size, prev_size;

	if (!regs || !n_regs) return 1;

	prev_addr = regs[0].address;
	prev_size = mbreg_size(&regs[0]);

	for (i=1; i<n_regs; ++i) {
		reg = regs+i;
		size = mbreg_size(reg);
		if (reg->type & MRTYPE_BLOCK) {
			size *= reg->n_block_entries;
		}

		if (reg->address < (prev_addr + (prev_size+1)/2)) {
			if (issue_addr) *issue_addr = reg->address;
			return 0;
		}

		prev_addr = reg->address;
		prev_size = size;
	}

	return 1;
}

extern int mbtest_regs_valid_data_type(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr)
{
	const struct mbreg_desc_s *reg;
	size_t i;

	if (!regs || !n_regs) return 1;

	for (i=0; i<n_regs; ++i) {
		reg = regs+i;
		switch (reg->type & MRTYPE_MASK) {
		case MRTYPE_U8:
		case MRTYPE_U16:
		case MRTYPE_U32:
		case MRTYPE_U64:
		case MRTYPE_I8:
		case MRTYPE_I16:
		case MRTYPE_I32:
		case MRTYPE_I64:
		case MRTYPE_F32:
		case MRTYPE_F64:
			break;
		default: /* Make sure there is one and only one type */
			if (issue_addr) *issue_addr = reg->address;
			return 0;
		}
	}

	return 1;
}

extern int mbtest_regs_valid_access(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr)
{
	const struct mbreg_desc_s *reg;
	size_t i;

	if (!regs || !n_regs) return 1;

	for (i=0; i<n_regs; ++i) {
		reg = regs+i;

		if (reg->access == 0) { /* Don't allow registers without any access */
			if (issue_addr) *issue_addr = reg->address;
			return 0;
		}

		if (reg->access & MRACC_R_MASK) {
			switch (reg->access & MRACC_R_MASK) {
			case MRACC_R_VAL:
				break;
			case MRACC_R_PTR:
				if (reg->read.pu8 == NULL) {
					if (issue_addr) *issue_addr = reg->address;
					return 0;
				}
				break;
			case MRACC_R_FN:
				if (reg->read.fu8 == NULL) {
					if (issue_addr) *issue_addr = reg->address;
					return 0;
				}
				break;
			default: /* Make sure there is one and only one access method */
				if (issue_addr) *issue_addr = reg->address;
				return 0;
			}
		} else { /* There should not be a read function */
			if (reg->access & MRACC_R_MASK) {
				if (issue_addr) *issue_addr = reg->address;
				return 0;
			}
			if (reg->read.pu8 != NULL) {
				if (issue_addr) *issue_addr = reg->address;
				return 0;
			}
		}

		if (reg->access & MRACC_W_MASK) {
			switch (reg->access & MRACC_W_MASK) {
			case MRACC_W_PTR:
				if (reg->write.pu8 == NULL) {
					if (issue_addr) *issue_addr = reg->address;
					return 0;
				}
				break;
			case MRACC_W_FN:
				if (reg->write.fu8 == NULL) {
					if (issue_addr) *issue_addr = reg->address;
					return 0;
				}
				break;
			default: /* Make sure there is one and only one access method */
				if (issue_addr) *issue_addr = reg->address;
				return 0;
			}
		} else { /* There should not be a write function */
			if (reg->access & MRACC_W_MASK) {
				if (issue_addr) *issue_addr = reg->address;
				return 0;
			}
			if (reg->write.pu8 != NULL) {
				if (issue_addr) *issue_addr = reg->address;
				return 0;
			}
		}
	}

	return 1;
}

extern int mbtest_regs_valid_block_access(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr)
{
	const struct mbreg_desc_s *reg;
	size_t i;

	if (!regs || !n_regs) return 1;

	for (i=0; i<n_regs; ++i) {
		reg = regs+i;
		if (reg->type & MRTYPE_BLOCK) {
			if ((reg->access & MRACC_R_MASK) && !(reg->access & MRACC_R_PTR)) {
				*issue_addr = reg->address;
				return 0;
			}
			if ((reg->access & MRACC_W_MASK) && !(reg->access & MRACC_W_PTR)) {
				*issue_addr = reg->address;
				return 0;
			}
		}
	}

	return 1;
}

extern int mbtest_regs_validate_all(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t *issue_addr)
{
	/* Run all register validation tests */
	if (!mbtest_regs_asc(regs, n_regs, issue_addr)) return 0;
	if (!mbtest_regs_valid_size(regs, n_regs, issue_addr)) return 0;
	if (!mbtest_regs_dont_overlap(regs, n_regs, issue_addr)) return 0;
	if (!mbtest_regs_valid_data_type(regs, n_regs, issue_addr)) return 0;
	if (!mbtest_regs_valid_access(regs, n_regs, issue_addr)) return 0;
	if (!mbtest_regs_valid_block_access(regs, n_regs, issue_addr)) return 0;

	return 1;
}

extern int mbtest_files_asc_and_unique(
	const struct mbfile_desc_s *files,
	size_t n_files,
	uint16_t *issue_file_no)
{
	size_t i;
	uint16_t file_no, prev_file_no;

	prev_file_no = files[0].file_no;

	for (i=1u; i<n_files; ++i) {
		file_no = files[i].file_no;
		if (file_no <= prev_file_no) {
			if (issue_file_no) *issue_file_no = file_no;
			return 0;
		}
		prev_file_no = file_no;
	}

	return 1;
}
