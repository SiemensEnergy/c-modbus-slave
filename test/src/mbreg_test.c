#include "test_lib.h"
#include <endian.h>
#include <mbreg.h>
#include <mbinst.h>

/* Test register size calculations for different data types */

TEST(mbreg_invalid_type_size_zero)
{
	const struct mbreg_desc_s reg = {
		.address=0x0000,
		.type=0, /* Invalid type */
		.access=MRACC_R_VAL,
		.read={.u16=0x1234}
	};

	size_t szb = mbreg_size(&reg);

	ASSERT_EQ(0u, szb);
}

TEST(mbreg_size_calculation_u8)
{
	struct mbreg_desc_s reg = {.type = MRTYPE_U8};

	size_t szb = mbreg_size(&reg);

	/* U8 registers are padded to 16-bit in Modbus protocol */
	ASSERT_EQ(2u, szb);
}

TEST(mbreg_size_calculation_u16)
{
	struct mbreg_desc_s reg = {.type = MRTYPE_U16};

	size_t szb = mbreg_size(&reg);
	ASSERT_EQ(2u, szb);
}

TEST(mbreg_size_calculation_u32)
{
	struct mbreg_desc_s reg = {.type = MRTYPE_U32};

	size_t szb = mbreg_size(&reg);
	ASSERT_EQ(4u, szb);
}

TEST(mbreg_size_calculation_u64)
{
	struct mbreg_desc_s reg = {.type = MRTYPE_U64};

	size_t szb = mbreg_size(&reg);
	ASSERT_EQ(8u, szb);
}

TEST(mbreg_size_calculation_f32)
{
	struct mbreg_desc_s reg = {.type = MRTYPE_F32};

	size_t szb = mbreg_size(&reg);
	ASSERT_EQ(4u, szb);
}

TEST(mbreg_size_calculation_f64)
{
	struct mbreg_desc_s reg = {.type = MRTYPE_F64};

	size_t szb = mbreg_size(&reg);
	ASSERT_EQ(8u, szb);
}

/* Test register descriptor lookup functionality */

TEST(mbreg_find_null_array_fails)
{
	const struct mbreg_desc_s *desc = mbreg_find_desc(NULL, 10, 0x0000);
	ASSERT_EQ(NULL, desc);
}

TEST(mbreg_find_zero_count_fails)
{
	uint16_t test_val = 0x1234;
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0000,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&test_val},
			.write={.pu16=&test_val}
		}
	};

	const struct mbreg_desc_s *desc = mbreg_find_desc(regs, 0, 0x0000);
	ASSERT_EQ(NULL, desc);
}

TEST(mbreg_find_exact_address_match)
{
	const struct mbreg_desc_s regs[] = {
		{.address = 0x0010, .type = MRTYPE_U16},
		{.address = 0x0020, .type = MRTYPE_U32},
		{.address = 0x0030, .type = MRTYPE_U16},
	};
	const size_t reg_count = sizeof regs / sizeof regs[0];

	const struct mbreg_desc_s *desc = mbreg_find_desc(regs, reg_count, 0x0020);

	ASSERT(desc);
	ASSERT_EQ(0x0020, desc->address);
	ASSERT_EQ(MRTYPE_U32, desc->type);
}

TEST(mbreg_find_nonexistent_address)
{
	const struct mbreg_desc_s regs[] = {
		{.address = 0x0010, .type = MRTYPE_U16},
		{.address = 0x0030, .type = MRTYPE_U16},
	};
	const size_t reg_count = sizeof regs / sizeof regs[0];

	const struct mbreg_desc_s *desc = mbreg_find_desc(regs, reg_count, 0x0020);
	ASSERT_EQ(NULL, desc);
}

TEST(mbreg_find_block_u16_register)
{
	const struct mbreg_desc_s regs[] = {
		{.address=0x100, .type=MRTYPE_U16 | MRTYPE_BLOCK, .n_block_entries=10},
	};
	const size_t reg_count = sizeof regs / sizeof regs[0];

	/* Should find register at base address */
	const struct mbreg_desc_s *desc = mbreg_find_desc(regs, reg_count, 0x100);
	ASSERT(desc);
	ASSERT_EQ(0x100, desc->address);

	/* Should find register within block range */
	desc = mbreg_find_desc(regs, reg_count, 0x105);
	ASSERT(desc);
	ASSERT_EQ(0x100, desc->address);

	/* Should find register at end of block */
	desc = mbreg_find_desc(regs, reg_count, 0x109);
	ASSERT(desc);
	ASSERT_EQ(0x100, desc->address);

	/* Should not find register past block end */
	desc = mbreg_find_desc(regs, reg_count, 0x10A);
	ASSERT_EQ(NULL, desc);
}

TEST(mbreg_find_block_u32_register)
{
	const struct mbreg_desc_s regs[] = {
		{.address=0x200, .type=MRTYPE_U32 | MRTYPE_BLOCK, .n_block_entries=5},
	};
	const size_t reg_count = sizeof regs / sizeof regs[0];

	/* Should find register at base address */
	const struct mbreg_desc_s *desc = mbreg_find_desc(regs, reg_count, 0x200);
	ASSERT(desc);
	ASSERT_EQ(0x200, desc->address);

	/* Should find register within block range */
	desc = mbreg_find_desc(regs, reg_count, 0x208);
	ASSERT(desc);
	ASSERT_EQ(0x200, desc->address);

	desc = mbreg_find_desc(regs, reg_count, 0x209);
	/* Within range, but it would overflow if treated as a 32-bit value */
	ASSERT(desc);
	ASSERT_EQ(0x200, desc->address);

	/* Should not find register past block end */
	desc = mbreg_find_desc(regs, reg_count, 0x20A);
	ASSERT_EQ(NULL, desc);
}

TEST(mbreg_read_only_write_fails)
{
	const struct mbreg_desc_s reg = {
		.address=0x0000,
		.type=MRTYPE_U16,
		.access=MRACC_R_VAL, /* Read-only */
		.read={.u16=0x1234}
	};
	uint8_t data[] = {0x56, 0x78};

	size_t n_written;
	enum mbstatus_e status = mbreg_write(&reg, 0x0000, 1u, data, &n_written);
	ASSERT_EQ(MB_DEV_FAIL, status);
}

TEST(mbreg_write_only_read_fails)
{
	uint16_t test_val = 0x1234;
	const struct mbreg_desc_s reg = {
		.address=0x0000,
		.type=MRTYPE_U16,
		.access=MRACC_W_PTR, /* Write-only */
		.write={.pu16=&test_val}
	};
	uint8_t res[MBPDU_DATA_SIZE_MAX];

	size_t result = mbreg_read(&reg, 0x0000, 1u, res, 0);
	ASSERT(result == 0u);
}

static int always_locked(void)
{
	return 1; /* Always locked */
}
TEST(mbreg_read_locked_fails)
{
	const struct mbreg_desc_s reg = {
		.address=0x0000,
		.type=MRTYPE_U16,
		.access=MRACC_R_VAL,
		.read={.u16=0x1234},
		.rlock_cb=always_locked
	};
	uint8_t res[MBPDU_DATA_SIZE_MAX];

	size_t result = mbreg_read(&reg, 0x0000, 1u, res, 0);
	ASSERT_EQ(MBREG_READ_LOCKED, result);
}

TEST(mbreg_write_locked_fails)
{
	uint16_t test_val = 0x1234;
	const struct mbreg_desc_s reg = {
		.address=0x0000,
		.type=MRTYPE_U16,
		.access=MRACC_W_PTR,
		.write={.pu16=&test_val},
		.wlock_cb=always_locked
	};
	uint8_t data[] = {0x56, 0x78};

	size_t n_written = mbreg_write_allowed(&reg, 0x0000, 0x0000, 1u, data);
	ASSERT_EQ(0u, n_written);
	ASSERT_EQ(0x1234u, test_val);
}

static enum mbstatus_e failing_write_fn(uint16_t value)
{
	(void)value;
	return MB_BUSY; /* Always fail */
}
TEST(mbreg_function_write_failure)
{
	const struct mbreg_desc_s reg = {
		.address=0x0000,
		.type=MRTYPE_U16,
		.access=MRACC_W_FN,
		.write={.fu16=failing_write_fn}
	};
	uint8_t data[] = {0x12, 0x34};

	size_t n_written;
	enum mbstatus_e status = mbreg_write(&reg, 0x0000, 1u, data, &n_written);
	ASSERT_EQ(MB_BUSY, status);
}

TEST(mbreg_partial_reg_read_works)
{
	const struct mbreg_desc_s reg = {
		.address=0x0000,
		.type=MRTYPE_U32,
		.access=MRACC_R_VAL,
		.read={.u32=0x12345678}
	};
	uint8_t res[MBPDU_DATA_SIZE_MAX];

	size_t result = mbreg_read(&reg, 0x0000, 1u, res, 0); /* Read first part of register */
	ASSERT(result > 0u);
	ASSERT_EQ(0x1234, betou16(res));

	result = mbreg_read(&reg, 0x0001, 1u, res, 0); /* Read second part of register */
	ASSERT(result > 0u);
	ASSERT_EQ(0x5678, betou16(res));
}

TEST(mbreg_partial_reg_write_works)
{
	uint32_t reg_val = 0u;
	const struct mbreg_desc_s reg = {
		.address=0x0000,
		.type=MRTYPE_U32,
		.access=MRACC_W_PTR,
		.write={.pu32=&reg_val}
	};

	uint8_t buf[2];
	size_t n_written;

	buf[0] = 0x12;
	buf[1] = 0x34;
	enum mbstatus_e status = mbreg_write(&reg, 0x0000, 1u, buf, &n_written);
	ASSERT_EQ(MB_OK, status);
	ASSERT_EQ(1u, n_written);
	ASSERT_EQ(0x12340000, reg_val);

	buf[0] = 0x56;
	buf[1] = 0x78;
	status = mbreg_write(&reg, 0x0001, 1u, buf, &n_written);
	ASSERT_EQ(MB_OK, status);
	ASSERT_EQ(1u, n_written);
	ASSERT_EQ(0x12345678, reg_val);
}

TEST_MAIN(
	mbreg_invalid_type_size_zero,
	mbreg_size_calculation_u8,
	mbreg_size_calculation_u16,
	mbreg_size_calculation_u32,
	mbreg_size_calculation_u64,
	mbreg_size_calculation_f32,
	mbreg_size_calculation_f64,
	mbreg_find_null_array_fails,
	mbreg_find_zero_count_fails,
	mbreg_find_exact_address_match,
	mbreg_find_nonexistent_address,
	mbreg_find_block_u16_register,
	mbreg_find_block_u32_register,
	mbreg_read_only_write_fails,
	mbreg_write_only_read_fails,
	mbreg_read_locked_fails,
	mbreg_write_locked_fails,
	mbreg_function_write_failure,
	mbreg_partial_reg_read_works,
	mbreg_partial_reg_write_works
);
