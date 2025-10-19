#include "test_lib.h"
#include <mbtest.h>
#include <mbcoil.h>
#include <mbreg.h>

/* ========== COIL VALIDATION TESTS ========== */

/* Test coil ascending order validation failures */
TEST(mbtest_coils_not_ascending_fails)
{
	uint8_t test_val1 = 0x55;
	uint8_t test_val2 = 0xAA;

	/* Coils not in ascending order */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0002,
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val1, .ix=0}
		},
		{
			.address=0x0001, /* This should come before 0x0002 */
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val2, .ix=1}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_asc(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_coils_duplicate_address_fails)
{
	uint8_t test_val1 = 0x55;
	uint8_t test_val2 = 0xAA;

	/* Duplicate coil addresses */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val1, .ix=0}
		},
		{
			.address=0x0001, /* Duplicate address */
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val2, .ix=1}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_no_duplicates(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the duplicate address */
}

/* Test coil bit index overflow validation */
TEST(mbtest_coils_bit_index_overflow_read_fails)
{
	uint8_t test_val = 0x55;

	/* Coil with invalid bit index for read */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val, .ix=8} /* Invalid bit index > 7 */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_bit_index(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_coils_bit_index_overflow_write_fails)
{
	uint8_t test_val = 0x55;

	/* Coil with invalid bit index for write */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_W_PTR,
			.write={.ptr=&test_val, .ix=15} /* Invalid bit index > 7 */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_bit_index(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_coils_bit_index_boundary_valid)
{
	uint8_t test_val = 0x55;

	/* Coil with valid boundary bit indices */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_RW_PTR,
			.read={.ptr=&test_val, .ix=0}, /* Valid: minimum index */
			.write={.ptr=&test_val, .ix=7} /* Valid: maximum index */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_bit_index(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(1, result); /* Should pass validation */
}

/* Test coil access validation failures */
TEST(mbtest_coils_no_access_fails)
{
	uint8_t test_val = 0x55;

	/* Coil with no access rights */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=0, /* No access */
			.read={.ptr=&test_val, .ix=0}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_access(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_coils_invalid_read_ptr_fails)
{
	/* Coil claims pointer read access but has NULL pointer */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_R_PTR,
			.read={.ptr=NULL, .ix=0} /* NULL pointer */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_access(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_coils_invalid_write_ptr_fails)
{
	/* Coil claims pointer write access but has NULL pointer */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_W_PTR,
			.write={.ptr=NULL, .ix=0} /* NULL pointer */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_access(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_coils_invalid_read_function_fails)
{
	/* Coil claims function read access but has NULL function */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_R_FN,
			.read={.fn=NULL} /* NULL function */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_access(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_coils_invalid_write_function_fails)
{
	/* Coil claims function write access but has NULL function */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_W_FN,
			.write={.fn=NULL} /* NULL function */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_access(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

/* Test valid coil configurations */
TEST(mbtest_coils_valid_value_access)
{
	/* Coil with valid value access */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_R_VAL,
			.read={.val=1} /* Valid constant value */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_access(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(1, result); /* Should pass validation */
}

/* Test helper functions for coil tests */
static int test_coil_read_fn(void)
{
	return 1;
}

static enum mbstatus_e test_coil_write_fn(int value)
{
	(void)value; /* Suppress unused parameter warning */
	return MB_OK;
}

TEST(mbtest_coils_valid_function_access)
{
	/* Coil with valid function access */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_RW_FN,
			.read={.fn=test_coil_read_fn},
			.write={.fn=test_coil_write_fn}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_access(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(1, result); /* Should pass validation */
}

/* Test comprehensive validation functions */
TEST(mbtest_coils_validate_all_fails_on_any_error)
{
	uint8_t test_val = 0x55;

	/* Coils with multiple errors */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0002,
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val, .ix=0}
		},
		{
			.address=0x0001, /* Not ascending order */
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val, .ix=8} /* Invalid bit index */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_validate_all(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(0, result); /* Should fail due to multiple errors */
	/* The function will report the first error it encounters */
}

TEST(mbtest_coils_validate_all_passes_valid_config)
{
	uint8_t test_val1 = 0x55;
	uint8_t test_val2 = 0xAA;

	/* Valid coil configuration */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_RW_PTR,
			.read={.ptr=&test_val1, .ix=0},
			.write={.ptr=&test_val1, .ix=0}
		},
		{
			.address=0x0002,
			.access=MCACC_R_FN,
			.read={.fn=test_coil_read_fn}
		},
		{
			.address=0x0003,
			.access=MCACC_R_VAL,
			.read={.val=1}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_validate_all(coils, sizeof coils / sizeof coils[0], &issue_addr);

	ASSERT_EQ(1, result); /* Should pass all validations */
}

/* Test edge cases */
TEST(mbtest_coils_null_issue_addr_works)
{
	uint8_t test_val = 0x55;

	/* Invalid coil configuration */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val, .ix=8} /* Invalid bit index */
		}
	};

	/* Should work even with NULL issue_addr pointer */
	int result = mbtest_coils_valid_bit_index(coils, sizeof coils / sizeof coils[0], NULL);
	ASSERT_EQ(0, result); /* Should still fail validation */
}

TEST(mbtest_coils_empty_array_valid)
{
	uint16_t issue_addr = 0;

	/* Empty coil arrays should be considered valid */
	ASSERT_EQ(1, mbtest_coils_asc(NULL, 0, &issue_addr));
	ASSERT_EQ(1, mbtest_coils_valid_access(NULL, 0, &issue_addr));
	ASSERT_EQ(1, mbtest_coils_valid_bit_index(NULL, 0, &issue_addr));
	ASSERT_EQ(1, mbtest_coils_no_duplicates(NULL, 0, &issue_addr));
	ASSERT_EQ(1, mbtest_coils_validate_all(NULL, 0, &issue_addr));
}

TEST(mbtest_single_coil_validation)
{
	uint8_t test_val = 0x55;

	/* Single valid coil */
	const struct mbcoil_desc_s coils[] = {
		{
			.address=0x0001,
			.access=MCACC_RW_PTR,
			.read={.ptr=&test_val, .ix=3},
			.write={.ptr=&test_val, .ix=3}
		}
	};

	uint16_t issue_addr = 0;

	/* All validations should pass for single valid coil */
	ASSERT_EQ(1, mbtest_coils_asc(coils, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_coils_valid_access(coils, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_coils_valid_bit_index(coils, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_coils_no_duplicates(coils, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_coils_validate_all(coils, 1, &issue_addr));
}

/* Test the critical bit index overflow issue that was found in code review */
TEST(mbtest_coils_bit_index_prevents_buffer_overflow)
{
	uint8_t test_val = 0x55;

	/* Test various invalid bit indices that could cause buffer overflow */
	const struct mbcoil_desc_s dangerous_coils[] = {
		{
			.address=0x0001,
			.access=MCACC_R_PTR,
			.read={.ptr=&test_val, .ix=255} /* Extreme overflow */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_coils_valid_bit_index(dangerous_coils, 1, &issue_addr);

	ASSERT_EQ(0, result); /* Must catch this dangerous configuration */
	ASSERT_EQ(0x0001, issue_addr);

	/* Test edge case: ix = 8 (first invalid value) */
	const struct mbcoil_desc_s edge_coils[] = {
		{
			.address=0x0002,
			.access=MCACC_W_PTR,
			.write={.ptr=&test_val, .ix=8} /* Just over the limit */
		}
	};

	result = mbtest_coils_valid_bit_index(edge_coils, 1, &issue_addr);
	ASSERT_EQ(0, result); /* Must catch ix=8 as invalid */
	ASSERT_EQ(0x0002, issue_addr);
}

/* ========== REGISTER VALIDATION TESTS ========== */

/* Test register ascending order validation failures */
TEST(mbtest_regs_not_ascending_fails)
{
	uint16_t test_val1 = 0x1234;
	uint16_t test_val2 = 0x5678;

	/* Registers not in ascending order */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0002,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val1}
		},
		{
			.address=0x0001, /* This should come before 0x0002 */
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val2}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_asc(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_regs_duplicate_address_fails)
{
	uint16_t test_val1 = 0x1234;
	uint16_t test_val2 = 0x5678;

	/* Duplicate addresses */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val1}
		},
		{
			.address=0x0001, /* Duplicate address */
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val2}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_asc(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the duplicate address */
}

/* Test register overlap validation failures */
TEST(mbtest_regs_overlap_fails)
{
	uint16_t test_val1 = 0x1234;
	uint32_t test_val2 = 0x56789ABC;

	/* Registers that overlap - U32 at 0x0001 overlaps with U16 at 0x0002 */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=MRTYPE_U32, /* Takes 2 registers: 0x0001, 0x0002 */
			.access=MRACC_R_PTR,
			.read={.pu32 = &test_val2}
		},
		{
			.address=0x0002, /* Overlaps with the U32 above */
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val1}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_dont_overlap(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0002, issue_addr); /* Should report the overlapping address */
}

/* Test invalid data type validation failures */
TEST(mbtest_regs_invalid_data_type_fails)
{
	uint16_t test_val = 0x1234;

	/* Register with invalid data type */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=0x1000, /* Invalid type */
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_valid_data_type(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

/* Test invalid access validation failures */
TEST(mbtest_regs_no_access_fails)
{
	uint16_t test_val = 0x1234;

	/* Register with no access rights */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=MRTYPE_U16,
			.access=0, /* No access */
			.read={.pu16=&test_val}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_valid_access(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_regs_invalid_read_access_fails)
{
	uint16_t test_val = 0x1234;

	/* Register claims read access but has invalid read configuration */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR, /* Claims pointer read access */
			.read={.pu16=NULL} /* But pointer is NULL */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_valid_access(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_regs_invalid_write_access_fails)
{
	uint16_t test_val = 0x1234;

	/* Register claims write access but has invalid write configuration */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=MRTYPE_U16,
			.access=MRACC_W_PTR, /* Claims pointer write access */
			.write={.pu16=NULL} /* But pointer is NULL */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_valid_access(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

TEST(mbtest_regs_function_access_without_function_fails)
{
	/* Register claims function access but has no function */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=MRTYPE_U16,
			.access=MRACC_R_FN, /* Claims function read access */
			.read={.fu16 = NULL} /* But function is NULL */
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_valid_access(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

/* Test block access validation failures */
TEST(mbtest_regs_block_invalid_access_fails)
{
	uint16_t test_val = 0x1234;

	/* block register with invalid access pattern (function instead of pointer) */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x1000,
			.type=MRTYPE_U16 | MRTYPE_BLOCK,
			.access=MRACC_R_FN, /* block requires pointer access, not function */
			.read={.fu16 = NULL},
			.n_block_entries = 10
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_valid_block_access(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x1000, issue_addr); /* Should report the problematic address */
}

/* Test valid size validation failures */
TEST(mbtest_regs_zero_size_fails)
{
	/* Register with type that results in zero size */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=0, /* Invalid type that gives zero size */
			.access=MRACC_R_VAL,
			.read={.u16 = 0x1234}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_valid_size(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result);
	ASSERT_EQ(0x0001, issue_addr); /* Should report the problematic address */
}

/* Test edge cases with NULL issue_addr pointer */
TEST(mbtest_regs_null_issue_addr_works)
{
	uint16_t test_val1 = 0x1234;
	uint16_t test_val2 = 0x5678;

	/* Invalid register configuration */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0002,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val1}
		},
		{
			.address=0x0001, /* Not ascending */
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val2}
		}
	};

	/* Should work even with NULL issue_addr pointer */
	int result = mbtest_regs_asc(regs, sizeof regs / sizeof regs[0], NULL);
	ASSERT_EQ(0, result); /* Should still fail validation */
}

/* Test with single register (edge case) */
TEST(mbtest_single_register_validation)
{
	uint16_t test_val = 0x1234;

	/* Single valid register */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&test_val},
			.write={.pu16=&test_val}
		}
	};

	uint16_t issue_addr = 0;

	/* All validations should pass for single valid register */
	ASSERT_EQ(1, mbtest_regs_asc(regs, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_regs_valid_size(regs, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_regs_dont_overlap(regs, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_regs_valid_data_type(regs, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_regs_valid_access(regs, 1, &issue_addr));
	ASSERT_EQ(1, mbtest_regs_valid_block_access(regs, 1, &issue_addr));
}

/* Test comprehensive register validation */
TEST(mbtest_regs_validate_all_fails_on_any_error)
{
	uint16_t test_val1 = 0x1234;
	uint16_t test_val2 = 0x5678;

	/* Registers with multiple errors */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0002,
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val1}
		},
		{
			.address=0x0001, /* Not ascending order */
			.type=MRTYPE_U16,
			.access=MRACC_R_PTR,
			.read={.pu16=&test_val2}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_validate_all(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(0, result); /* Should fail due to ordering error */
	ASSERT_EQ(0x0001, issue_addr); /* Should report the first error */
}

TEST(mbtest_regs_validate_all_passes_valid_config)
{
	uint16_t test_val1 = 0x1234;
	uint32_t test_val2 = 0x56789ABC;

	/* Valid register configuration */
	const struct mbreg_desc_s regs[] = {
		{
			.address=0x0001,
			.type=MRTYPE_U16,
			.access=MRACC_RW_PTR,
			.read={.pu16=&test_val1},
			.write={.pu16=&test_val1}
		},
		{
			.address=0x0010,
			.type=MRTYPE_U32,
			.access=MRACC_R_PTR,
			.read={.pu32=&test_val2}
		}
	};

	uint16_t issue_addr = 0;
	int result = mbtest_regs_validate_all(regs, sizeof regs / sizeof regs[0], &issue_addr);

	ASSERT_EQ(1, result); /* Should pass all validations */
}

TEST_MAIN(
	mbtest_coils_not_ascending_fails,
	mbtest_coils_duplicate_address_fails,
	mbtest_coils_bit_index_overflow_read_fails,
	mbtest_coils_bit_index_overflow_write_fails,
	mbtest_coils_bit_index_boundary_valid,
	mbtest_coils_no_access_fails,
	mbtest_coils_invalid_read_ptr_fails,
	mbtest_coils_invalid_write_ptr_fails,
	mbtest_coils_invalid_read_function_fails,
	mbtest_coils_invalid_write_function_fails,
	mbtest_coils_valid_value_access,
	mbtest_coils_valid_function_access,
	mbtest_coils_validate_all_fails_on_any_error,
	mbtest_coils_validate_all_passes_valid_config,
	mbtest_coils_null_issue_addr_works,
	mbtest_coils_empty_array_valid,
	mbtest_single_coil_validation,
	mbtest_coils_bit_index_prevents_buffer_overflow,
	mbtest_regs_not_ascending_fails,
	mbtest_regs_duplicate_address_fails,
	mbtest_regs_overlap_fails,
	mbtest_regs_invalid_data_type_fails,
	mbtest_regs_no_access_fails,
	mbtest_regs_invalid_read_access_fails,
	mbtest_regs_invalid_write_access_fails,
	mbtest_regs_function_access_without_function_fails,
	mbtest_regs_block_invalid_access_fails,
	mbtest_regs_zero_size_fails,
	mbtest_regs_null_issue_addr_works,
	mbtest_single_register_validation,
	mbtest_regs_validate_all_fails_on_any_error,
	mbtest_regs_validate_all_passes_valid_config
)
