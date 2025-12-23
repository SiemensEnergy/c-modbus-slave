#include "test_lib.h"
#include <endian.h>
#include <mbinst.h>
#include <mbpdu.h>

TEST(mbpdu_file_read_works)
{
	const struct mbreg_desc_s file1[] = {
		{
			.address=0x09u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xDEADu},
		},
		{
			.address=0x0Au,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xBEEFu},
		}
	};
	const struct mbreg_desc_s file2[] = {
		{
			.address=0x01u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0x1234u},
		},
		{
			.address=0x02u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xABCDu},
		}
	};
	const struct mbfile_desc_s files[] = {
		{
			.file_no=0x03u,
			.records=file1,
			.n_records=sizeof file1 / sizeof file1[0],
		},
		{
			.file_no=0x04u,
			.records=file2,
			.n_records=sizeof file2 / sizeof file2[0],
		}
	};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_FILE_RECORD,
		0x0E, /* Byte count */
		0x06, /* Sub-req 1, Ref type */
		0x00, 0x04, /* Sub-req 1, File number */
		0x00, 0x01, /* Sub-req 1, Record number (register address) */
		0x00, 0x02, /* Sub-req 1, Record length (n registers to read) */
		0x06, /* Sub-req 2, Ref type */
		0x00, 0x03, /* Sub-req 2, File number */
		0x00, 0x09, /* Sub-req 2, Record number */
		0x00, 0x02, /* Sub-req 2, Record length */
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_FILE_RECORD, res[0]);
	ASSERT_EQ(14u, res_size);
	ASSERT_EQ(12u, res[1]); /* Byte count */

	ASSERT_EQ(5u, res[2]); /* Sub-req 1, File resp. length */
	ASSERT_EQ(0x06u, res[3]); /* Sub-req 1, File resp. length */
	ASSERT_EQ(0x12u, res[4]); /* Sub-req 1, Data hi */
	ASSERT_EQ(0x34u, res[5]); /* Sub-req 1, Data lo */
	ASSERT_EQ(0xABu, res[6]); /* Sub-req 1, Data hi */
	ASSERT_EQ(0xCDu, res[7]); /* Sub-req 1, Data lo */

	ASSERT_EQ(5u, res[8]); /* Sub-req 2, File resp. length */
	ASSERT_EQ(0x06u, res[9]); /* Sub-req 2, File resp. length */
	ASSERT_EQ(0xDEu, res[10]); /* Sub-req 2, Data hi */
	ASSERT_EQ(0xADu, res[11]); /* Sub-req 2, Data lo */
	ASSERT_EQ(0xBEu, res[12]); /* Sub-req 2, Data hi */
	ASSERT_EQ(0xEFu, res[13]); /* Sub-req 2, Data lo */
}

TEST(mbpdu_file_write_works)
{
	uint16_t val1 = 0xDEADu;
	uint16_t val2 = 0xBEEFu;
	uint16_t val3 = 0x1234u;
	uint16_t val4 = 0xABCDu;
	uint16_t val5 = 0xFEDCu;
	const struct mbreg_desc_s file1[] = {
		{
			.address=0x09u,
			.type=MRTYPE_U16,
			.access=MRACC_W_PTR,
			.write={.pu16=&val1},
		},
		{
			.address=0x0Au,
			.type=MRTYPE_U16,
			.access=MRACC_W_PTR,
			.write={.pu16=&val2},
		}
	};
	const struct mbreg_desc_s file2[] = {
		{
			.address=0x07u,
			.type=MRTYPE_U16,
			.access=MRACC_W_PTR,
			.write={.pu16=&val3},
		},
		{
			.address=0x08u,
			.type=MRTYPE_U16,
			.access=MRACC_W_PTR,
			.write={.pu16=&val4},
		},
		{
			.address=0x09u,
			.type=MRTYPE_U16,
			.access=MRACC_W_PTR,
			.write={.pu16=&val5},
		}
	};
	const struct mbfile_desc_s files[] = {
		{
			.file_no=0x03u,
			.records=file1,
			.n_records=sizeof file1 / sizeof file1[0],
		},
		{
			.file_no=0x04u,
			.records=file2,
			.n_records=sizeof file2 / sizeof file2[0],
		}
	};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_FILE_RECORD,
		0x0D, /* Byte count */
		0x06, /* Sub-req 1, Ref type */
		0x00, 0x04, /* Sub-req 1, File number */
		0x00, 0x07, /* Sub-req 1, Record number (register address) */
		0x00, 0x03, /* Sub-req 1, Record length (n registers to read) */
		0x06, 0xAF, /* Sub-req 1, Data (reg 0x07)*/
		0x04, 0xBE, /* Sub-req 1, Data (reg 0x08)*/
		0x10, 0x0D, /* Sub-req 1, Data (reg 0x09)*/
	};

	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_WRITE_FILE_RECORD, res[0]);
	ASSERT_EQ(15u, res_size);
	ASSERT_EQ(13u, res[1]); /* Byte count */

	ASSERT_EQ(0x06u, res[2]); /* Sub-req 1, Ref. type */
	ASSERT_EQ(0x00u, res[3]); /* Sub-req 1, File number hi */
	ASSERT_EQ(0x04u, res[4]); /* Sub-req 1, File number lo */
	ASSERT_EQ(0x00u, res[5]); /* Sub-req 1, Record number hi */
	ASSERT_EQ(0x07u, res[6]); /* Sub-req 1, Record number lo */
	ASSERT_EQ(0x00u, res[7]); /* Sub-req 1, Record length hi */
	ASSERT_EQ(0x03u, res[8]); /* Sub-req 1, Record length lo */
	ASSERT_EQ(0x06u, res[9]); /* Sub-req 2, Data (reg 0x07) hi */
	ASSERT_EQ(0xAFu, res[10]); /* Sub-req 2, Data (reg 0x07) lo */
	ASSERT_EQ(0x04u, res[11]); /* Sub-req 2, Data (reg 0x08) hi */
	ASSERT_EQ(0xBEu, res[12]); /* Sub-req 2, Data (reg 0x08) lo */
	ASSERT_EQ(0x10u, res[13]); /* Sub-req 2, Data (reg 0x09) hi */
	ASSERT_EQ(0x0Du, res[14]); /* Sub-req 2, Data (reg 0x09) lo */

	ASSERT_EQ(0x06AFu, val3);
	ASSERT_EQ(0x04BEu, val4);
	ASSERT_EQ(0x100Du, val5);
}

TEST(mbpdu_file_read_too_short_request)
{
	const struct mbfile_desc_s files[] = {0};
	struct mbinst_s inst = {
		.files=files,
		.n_files=0u,
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {MBFC_READ_FILE_RECORD, 0x07}; /* Missing sub-request data */
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_file_read_invalid_byte_count)
{
	const struct mbreg_desc_s file1[] = {
		{
			.address=0x09u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xDEADu},
		},
		{
			.address=0x0Au,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xBEEFu},
		}
	};
	const struct mbfile_desc_s files[] = {
		{
			.file_no=0x01u,
			.records=file1,
			.n_records=sizeof file1 / sizeof file1[0],
		},
	};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	/* Byte count too small */
	uint8_t pdu_data1[] = {
		MBFC_READ_FILE_RECORD,
		0x06, /* Should be at least 7 */
		0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data1, sizeof pdu_data1, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);

	/* Byte count too large */
	uint8_t pdu_data2[] = {
		MBFC_READ_FILE_RECORD,
		0xF6, /* Should be at most 0xF5 */
		0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01
	};

	res_size = mbpdu_handle_req(&inst, pdu_data2, sizeof pdu_data2, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);

	/* Byte count doesn't match actual data length */
	uint8_t pdu_data3[] = {
		MBFC_READ_FILE_RECORD,
		0x0E, /* Says 14 bytes but only 7 provided */
		0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01
	};

	res_size = mbpdu_handle_req(&inst, pdu_data3, sizeof pdu_data3, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);

	/* Byte count not multiple of 7 */
	uint8_t pdu_data4[] = {
		MBFC_READ_FILE_RECORD,
		0x08, /* Should be multiple of 7 (size of a sub-request) */
		0x06, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00
	};

	res_size = mbpdu_handle_req(&inst, pdu_data4, sizeof pdu_data4, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_file_read_invalid_reference_type)
{
	const struct mbreg_desc_s file1[] = {
		{
			.address=0x09u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xDEADu},
		},
		{
			.address=0x0Au,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xBEEFu},
		}
	};
	const struct mbfile_desc_s files[] = {
		{
			.file_no=0x01u,
			.records=file1,
			.n_records=sizeof file1 / sizeof file1[0],
		},
	};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_FILE_RECORD,
		0x07,
		0x05, /* Invalid ref type, should be 0x06 */
		0x00, 0x01, 0x00, 0x01, 0x00, 0x01
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_file_read_zero_file_number)
{
	const struct mbreg_desc_s file1[] = {
		{
			.address=0x09u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xDEADu},
		},
		{
			.address=0x0Au,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xBEEFu},
		}
	};
	const struct mbfile_desc_s files[] = {
		{
			.file_no=0x01u,
			.records=file1,
			.n_records=sizeof file1 / sizeof file1[0],
		},
	};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_FILE_RECORD,
		0x07,
		0x06,
		0x00, 0x00, /* File number 0 is invalid */
		0x00, 0x01,
		0x00, 0x01
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_file_read_zero_record_length)
{
	const struct mbreg_desc_s file1[] = {
		{
			.address=0x09u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xDEADu},
		},
		{
			.address=0x0Au,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0xBEEFu},
		}
	};
	const struct mbfile_desc_s files[] = {
		{
			.file_no=0x01u,
			.records=file1,
			.n_records=sizeof file1 / sizeof file1[0],
		},
	};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_FILE_RECORD,
		0x07,
		0x06,
		0x00, 0x01,
		0x00, 0x01,
		0x00, 0x00 /* Record length 0 is invalid */
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_file_read_nonexistent_file)
{
	const struct mbreg_desc_s file1[] = {
		{
			.address=0x01u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0x1234u},
		}
	};
	const struct mbfile_desc_s files[] = {
		{
			.file_no=0x01u,
			.records=file1,
			.n_records=sizeof file1 / sizeof file1[0],
		}
	};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_FILE_RECORD,
		0x07,
		0x06,
		0x00, 0x02, /* File 2 doesn't exist */
		0x00, 0x01,
		0x00, 0x01
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_file_read_response_too_large)
{
	const struct mbreg_desc_s file1[] = {
		{
			.address=0x01u,
			.type=MRTYPE_U16,
			.access=MRACC_R_VAL,
			.read={.u16=0x1234u},
		}
	};
	const struct mbfile_desc_s files[] = {
		{
			.file_no=0x01u,
			.records=file1,
			.n_records=sizeof file1 / sizeof file1[0],
		}
	};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_READ_FILE_RECORD,
		0x07,
		0x06,
		0x00, 0x01,
		0x00, 0x01,
		0x00, 0x7A /* Record length 122 * 2 + 2 header = 246 bytes, too large for response */
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_READ_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_file_write_too_short_request)
{
	const struct mbfile_desc_s files[] = {0};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_FILE_RECORD,
		0x08 /* Too short, need at least 9 bytes */
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_WRITE_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_file_write_invalid_byte_count)
{
	const struct mbfile_desc_s files[] = {0};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	/* Byte count too small */
	uint8_t pdu_data1[] = {
		MBFC_WRITE_FILE_RECORD,
		0x08, /* Should be at least 9 */
		0x06,
		0x00, 0x01,
		0x00, 0x01,
		0x00, 0x01,
		0x12, 0x34
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data1, sizeof pdu_data1, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_WRITE_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);

	/* Byte count doesn't match actual data length */
	uint8_t pdu_data2[] = {
		MBFC_WRITE_FILE_RECORD,
		0x0A, /* Says 10 bytes but only 9 provided */
		0x06,
		0x00, 0x01,
		0x00, 0x01,
		0x00, 0x01,
		0x12, 0x34
	};

	res_size = mbpdu_handle_req(&inst, pdu_data2, sizeof pdu_data2, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_WRITE_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_file_write_insufficient_data)
{
	const struct mbfile_desc_s files[] = {0};
	struct mbinst_s inst = {
		.files=files,
		.n_files=sizeof files / sizeof files[0],
	};
	mbinst_init(&inst);

	uint8_t pdu_data[] = {
		MBFC_WRITE_FILE_RECORD,
		0x08,
		0x06, 0x00, 0x01, 0x00, 0x01,
		0x00, 0x02, /* Record length 2, but only 1 byte of data follows */
		0x12
	};
	uint8_t res[MBPDU_SIZE_MAX];

	size_t res_size = mbpdu_handle_req(&inst, pdu_data, sizeof pdu_data, res);
	ASSERT_EQ(2u, res_size);
	ASSERT_EQ(MBFC_WRITE_FILE_RECORD | MB_ERR_FLG, res[0]);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST_MAIN(
	mbpdu_file_read_works,
	mbpdu_file_write_works,
	mbpdu_file_read_too_short_request,
	mbpdu_file_read_invalid_byte_count,
	mbpdu_file_read_invalid_reference_type,
	mbpdu_file_read_zero_file_number,
	mbpdu_file_read_zero_record_length,
	mbpdu_file_read_nonexistent_file,
	mbpdu_file_read_response_too_large,
	mbpdu_file_write_too_short_request,
	mbpdu_file_write_invalid_byte_count,
	mbpdu_file_write_insufficient_data
);
