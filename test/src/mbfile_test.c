/**
 * @file mbfile_test.c
 * @brief Unit tests for Modbus file record operations (mbfile.c)
 * @author Generated Tests
 */

#include "test_lib.h"
#include <mbfile.h>
#include <mbreg.h>
#include <mbpdu.h>
#include <string.h>

TEST(mbfile_find_null_params)
{
	const struct mbfile_desc_s *result = mbfile_find(NULL, 0, 1);
	ASSERT_EQ(NULL, result);
}

TEST(mbfile_find_empty_array)
{
	const struct mbfile_desc_s files[] = {{0}};
	const struct mbfile_desc_s *result = mbfile_find(files, 0, 1);
	ASSERT_EQ(NULL, result);
}

TEST(mbfile_find_linear_search)
{
	/* Test with small array (uses linear search) */
	const struct mbreg_desc_s regs[] = {
		{.address=1, .type=MRTYPE_U16, .access=MRACC_R_VAL, .read={.u16=0x1234}}
	};
	const struct mbfile_desc_s files[] = {
		{.file_no=1, .records=regs, .n_records=1},
		{.file_no=3, .records=regs, .n_records=1},
		{.file_no=5, .records=regs, .n_records=1},
	};

	const struct mbfile_desc_s *result;

	/* Find existing files */
	result = mbfile_find(files, 3, 1);
	ASSERT(result != NULL);
	ASSERT_EQ(1u, result->file_no);

	result = mbfile_find(files, 3, 3);
	ASSERT(result != NULL);
	ASSERT_EQ(3u, result->file_no);

	result = mbfile_find(files, 3, 5);
	ASSERT(result != NULL);
	ASSERT_EQ(5u, result->file_no);

	/* Try to find non-existing file */
	result = mbfile_find(files, 3, 2);
	ASSERT_EQ(NULL, result);

	result = mbfile_find(files, 3, 6);
	ASSERT_EQ(NULL, result);
}

TEST(mbfile_find_binary_search)
{
	/* Test with large array (uses binary search) */
	const struct mbreg_desc_s regs[] = {
		{.address=1, .type=MRTYPE_U16, .access=MRACC_R_VAL, .read={.u16=0x1234}}
	};
	const struct mbfile_desc_s files[20] = {
		{.file_no=1, .records=regs, .n_records=1},
		{.file_no=2, .records=regs, .n_records=1},
		{.file_no=3, .records=regs, .n_records=1},
		{.file_no=4, .records=regs, .n_records=1},
		{.file_no=5, .records=regs, .n_records=1},
		{.file_no=6, .records=regs, .n_records=1},
		{.file_no=7, .records=regs, .n_records=1},
		{.file_no=8, .records=regs, .n_records=1},
		{.file_no=9, .records=regs, .n_records=1},
		{.file_no=10, .records=regs, .n_records=1},
		{.file_no=11, .records=regs, .n_records=1},
		{.file_no=12, .records=regs, .n_records=1},
		{.file_no=13, .records=regs, .n_records=1},
		{.file_no=14, .records=regs, .n_records=1},
		{.file_no=15, .records=regs, .n_records=1},
		{.file_no=16, .records=regs, .n_records=1},
		{.file_no=17, .records=regs, .n_records=1},
		{.file_no=18, .records=regs, .n_records=1},
		{.file_no=19, .records=regs, .n_records=1},
		{.file_no=20, .records=regs, .n_records=1},
	};

	const struct mbfile_desc_s *result;

	/* Find files at different positions */
	result = mbfile_find(files, 20, 1);
	ASSERT(result != NULL);
	ASSERT_EQ(1u, result->file_no);

	result = mbfile_find(files, 20, 10);
	ASSERT(result != NULL);
	ASSERT_EQ(10u, result->file_no);

	result = mbfile_find(files, 20, 20);
	ASSERT(result != NULL);
	ASSERT_EQ(20u, result->file_no);

	/* Try to find non-existing file */
	result = mbfile_find(files, 20, 21);
	ASSERT_EQ(NULL, result);

	result = mbfile_find(files, 20, 0);
	ASSERT_EQ(NULL, result);
}

TEST(mbfile_read_missing_first_record)
{
	const struct mbreg_desc_s regs[] = {
		{.address=5, .type=MRTYPE_U16, .access=MRACC_R_VAL, .read={.u16=0x1234}}
	};
	const struct mbfile_desc_s file = {
		.file_no=1,
		.records=regs,
		.n_records=1
	};

	uint8_t buffer[10];
	struct mbpdu_buf_s res = {.p=buffer, .size=0};

	/* Try to read from record 1 (doesn't exist, first record is 5) */
	enum mbfile_read_status_e status = mbfile_read(&file, 1, 2, &res);
	ASSERT_EQ(MBFILE_READ_ILLEGAL_ADDR, status);
}

TEST(mbfile_read_partial_records)
{
	const struct mbreg_desc_s regs[] = {
		{.address=1, .type=MRTYPE_U16, .access=MRACC_R_VAL, .read={.u16=0x1234}},
		{.address=3, .type=MRTYPE_U16, .access=MRACC_R_VAL, .read={.u16=0x5678}},
		/* Gap at address 2 and 4 */
	};
	const struct mbfile_desc_s file = {
		.file_no=1,
		.records=regs,
		.n_records=2
	};

	uint8_t buffer[10];
	struct mbpdu_buf_s res = {.p=buffer, .size=0};

	/* Read from 1 to 4 (should fill gaps with zeros) */
	enum mbfile_read_status_e status = mbfile_read(&file, 1, 4, &res);
	ASSERT_EQ(MBFILE_READ_OK, status);
	ASSERT_EQ(8u, res.size); /* 4 registers * 2 bytes */

	/* Check values: 0x1234, 0x0000, 0x5678, 0x0000 */
	ASSERT_EQ(0x12u, buffer[0]);
	ASSERT_EQ(0x34u, buffer[1]);
	ASSERT_EQ(0x00u, buffer[2]);
	ASSERT_EQ(0x00u, buffer[3]);
	ASSERT_EQ(0x56u, buffer[4]);
	ASSERT_EQ(0x78u, buffer[5]);
	ASSERT_EQ(0x00u, buffer[6]);
	ASSERT_EQ(0x00u, buffer[7]);
}

TEST(mbfile_read_no_access_registers)
{
	const struct mbreg_desc_s regs[] = {
		{.address=1, .type=MRTYPE_U16, .access=MRACC_R_VAL, .read={.u16=0x1234}},
		{.address=2, .type=MRTYPE_U16}, /* No access */
	};
	const struct mbfile_desc_s file = {
		.file_no=1,
		.records=regs,
		.n_records=2
	};

	uint8_t buffer[10];
	struct mbpdu_buf_s res = {.p=buffer, .size=0};

	/* Read both registers */
	enum mbfile_read_status_e status = mbfile_read(&file, 1, 2, &res);
	ASSERT_EQ(MBFILE_READ_OK, status);
	ASSERT_EQ(4u, res.size);

	/* Check values: 0x1234, 0x0000 (no access becomes zero) */
	ASSERT_EQ(0x12u, buffer[0]);
	ASSERT_EQ(0x34u, buffer[1]);
	ASSERT_EQ(0x00u, buffer[2]);
	ASSERT_EQ(0x00u, buffer[3]);
}

TEST(mbfile_read_null_buffer)
{
	const struct mbreg_desc_s regs[] = {
		{.address=1, .type=MRTYPE_U16, .access=MRACC_R_VAL, .read={.u16=0x1234}}
	};
	const struct mbfile_desc_s file = {
		.file_no=1,
		.records=regs,
		.n_records=1
	};

	/* Test with NULL buffer (should still validate and return OK) */
	enum mbfile_read_status_e status = mbfile_read(&file, 1, 1, NULL);
	ASSERT_EQ(MBFILE_READ_OK, status);
}

TEST(mbfile_write_allowed_missing_register)
{
	const struct mbreg_desc_s regs[] = {
		{.address=1, .type=MRTYPE_U16, .access=MRACC_W_PTR, .write={.pu16=(uint16_t[]){0}}}
	};
	const struct mbfile_desc_s file = {
		.file_no=1,
		.records=regs,
		.n_records=1
	};

	uint8_t data[] = {0x12, 0x34, 0x56, 0x78};

	/* Try to write to record 2 (doesn't exist) */
	enum mbstatus_e status = mbfile_write_allowed(&file, 2, 2, data);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, status);
}

TEST(mbfile_write_allowed_partial_success)
{
	uint16_t val1 = 0, val2 = 0;
	const struct mbreg_desc_s regs[] = {
		{.address=1, .type=MRTYPE_U16, .access=MRACC_W_PTR, .write={.pu16=&val1}},
		{.address=2, .type=MRTYPE_U16, .access=MRACC_W_PTR, .write={.pu16=&val2}},
		/* No register at address 3 */
	};
	const struct mbfile_desc_s file = {
		.file_no=1,
		.records=regs,
		.n_records=2
	};

	uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};

	/* Try to write 3 registers starting from 1 (should fail at address 3) */
	enum mbstatus_e status = mbfile_write_allowed(&file, 1, 3, data);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, status);
}

TEST(mbfile_write_success)
{
	uint16_t val1 = 0, val2 = 0;
	const struct mbreg_desc_s regs[] = {
		{.address=1, .type=MRTYPE_U16, .access=MRACC_W_PTR, .write={.pu16=&val1}},
		{.address=2, .type=MRTYPE_U16, .access=MRACC_W_PTR, .write={.pu16=&val2}},
	};
	const struct mbfile_desc_s file = {
		.file_no=1,
		.records=regs,
		.n_records=2
	};

	uint8_t data[] = {0x12, 0x34, 0x56, 0x78};

	/* Write 2 registers */
	enum mbstatus_e status = mbfile_write(&file, 1, 2, data);
	ASSERT_EQ(MB_OK, status);

	/* Check that values were written correctly */
	ASSERT_EQ(0x1234u, val1);
	ASSERT_EQ(0x5678u, val2);
}

TEST_MAIN(
	mbfile_find_null_params,
	mbfile_find_empty_array,
	mbfile_find_linear_search,
	mbfile_find_binary_search,
	mbfile_read_missing_first_record,
	mbfile_read_partial_records,
	mbfile_read_no_access_registers,
	mbfile_read_null_buffer,
	mbfile_write_allowed_missing_register,
	mbfile_write_allowed_partial_success,
	mbfile_write_success
);
