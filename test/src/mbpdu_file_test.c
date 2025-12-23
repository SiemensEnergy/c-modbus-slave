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

TEST_MAIN(
	mbpdu_file_read_works
);
