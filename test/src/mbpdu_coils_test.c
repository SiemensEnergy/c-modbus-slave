/*
 * PDU-level tests for Modbus coil and discrete input function codes.
 *
 * Covered function codes (from Modbus Application Protocol Specification V1.1b3):
 *   FC 01 (0x01) Read Coils           - sec 6.1
 *   FC 02 (0x02) Read Discrete Inputs - sec 6.2
 *   FC 05 (0x05) Write Single Coil    - sec 6.5
 *   FC 0F (0x0F) Write Multiple Coils - sec 6.11
 */

#include "test_lib.h"
#include <mbcoil.h>
#include <mbdef.h>
#include <mbinst.h>
#include <mbpdu.h>

/* =========================================================================
 * FC 01 Read Coils
 * Spec sec 6.1: Quantity range 1..2000. Coils packed LSB-first per byte.
 * Byte count = ceil(quantity / 8). Padding bits in last byte are zeroed.
 * Response: [FC][byte_count][coil_status...]
 * ========================================================================= */

TEST(mbpdu_read_coils_single_on)
{
	/* A single coil in the ON state must be returned with bit 0 set. */
	uint8_t coil_byte = 0x01u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_COILS, res[0]);
	ASSERT_EQ(3u, res_size);  /* FC + byte_count + 1 data byte */
	ASSERT_EQ(1u, res[1]);    /* byte count = 1 */
	ASSERT_EQ(0x01u, res[2]); /* coil 0 ON → bit 0 set */
}

TEST(mbpdu_read_coils_single_off)
{
	/* A single coil in the OFF state must be returned with bit 0 clear. */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0000u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(3u, res_size);
	ASSERT_EQ(1u, res[1]);
	ASSERT_EQ(0x00u, res[2]);
}

TEST(mbpdu_read_coils_lsb_first_packing)
{
	/*
	 * Spec sec 6.1: "The LSB of the first data byte contains the output
	 * addressed in the query. The other coils follow toward the high order end."
	 *
	 * Eight coils at addresses 0-7. Odd-addressed coils (1,3,5,7) are ON.
	 * Expected byte: bit1=1, bit3=1, bit5=1, bit7=1 → 0b10101010 = 0xAA.
	 */
	uint8_t coil_byte = 0xAAu;
	const struct mbcoil_desc_s coils[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}},
		{.address=1u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=1u}},
		{.address=2u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=2u}},
		{.address=3u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=3u}},
		{.address=4u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=4u}},
		{.address=5u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=5u}},
		{.address=6u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=6u}},
		{.address=7u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=7u}},
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=8u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x08u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(3u, res_size);
	ASSERT_EQ(1u, res[1]);    /* 8 coils → 1 byte */
	ASSERT_EQ(0xAAu, res[2]);
}

TEST(mbpdu_read_coils_partial_byte_zero_padded)
{
	/*
	 * Spec sec 6.1: "If the returned output quantity is not a multiple of eight,
	 * the remaining bits in the final data byte will be padded with zeros
	 * (toward the high order end of the byte)."
	 *
	 * Read 3 coils all ON → response byte must be 0x07 (bits 0,1,2 set, bits 3-7 zeroed).
	 */
	uint8_t coil_byte = 0xFFu;
	const struct mbcoil_desc_s coils[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}},
		{.address=1u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=1u}},
		{.address=2u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=2u}},
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=3u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x03u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(3u, res_size);
	ASSERT_EQ(1u, res[1]);    /* ceil(3/8) = 1 byte */
	ASSERT_EQ(0x07u, res[2]); /* bits 0,1,2 set; bits 3-7 padded with zero */
}

TEST(mbpdu_read_coils_multi_byte_response)
{
	/*
	 * Spec sec 6.1: coils spanning multiple bytes in the response.
	 * 16 coils at addresses 0-15. Even-addressed coils (0,2,4,6,8,10,12,14) are ON.
	 * First byte (coils 0-7):  bits 0,2,4,6 set → 0x55.
	 * Second byte (coils 8-15): bits 0,2,4,6 set → 0x55.
	 */
	uint8_t byte_lo = 0x55u;
	uint8_t byte_hi = 0x55u;
	const struct mbcoil_desc_s coils[] = {
		{.address= 0u, .access=MCACC_R_PTR, .read={.ptr=&byte_lo, .ix=0u}},
		{.address= 1u, .access=MCACC_R_PTR, .read={.ptr=&byte_lo, .ix=1u}},
		{.address= 2u, .access=MCACC_R_PTR, .read={.ptr=&byte_lo, .ix=2u}},
		{.address= 3u, .access=MCACC_R_PTR, .read={.ptr=&byte_lo, .ix=3u}},
		{.address= 4u, .access=MCACC_R_PTR, .read={.ptr=&byte_lo, .ix=4u}},
		{.address= 5u, .access=MCACC_R_PTR, .read={.ptr=&byte_lo, .ix=5u}},
		{.address= 6u, .access=MCACC_R_PTR, .read={.ptr=&byte_lo, .ix=6u}},
		{.address= 7u, .access=MCACC_R_PTR, .read={.ptr=&byte_lo, .ix=7u}},
		{.address= 8u, .access=MCACC_R_PTR, .read={.ptr=&byte_hi, .ix=0u}},
		{.address= 9u, .access=MCACC_R_PTR, .read={.ptr=&byte_hi, .ix=1u}},
		{.address=10u, .access=MCACC_R_PTR, .read={.ptr=&byte_hi, .ix=2u}},
		{.address=11u, .access=MCACC_R_PTR, .read={.ptr=&byte_hi, .ix=3u}},
		{.address=12u, .access=MCACC_R_PTR, .read={.ptr=&byte_hi, .ix=4u}},
		{.address=13u, .access=MCACC_R_PTR, .read={.ptr=&byte_hi, .ix=5u}},
		{.address=14u, .access=MCACC_R_PTR, .read={.ptr=&byte_hi, .ix=6u}},
		{.address=15u, .access=MCACC_R_PTR, .read={.ptr=&byte_hi, .ix=7u}},
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=16u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x10u}; /* qty=16 */
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(4u, res_size);   /* FC + byte_count + 2 data bytes */
	ASSERT_EQ(2u, res[1]);     /* ceil(16/8) = 2 bytes */
	ASSERT_EQ(0x55u, res[2]);  /* coils 0-7 */
	ASSERT_EQ(0x55u, res[3]);  /* coils 8-15 */
}

TEST(mbpdu_read_coils_zero_quantity_fails)
{
	/* Spec sec 6.1: quantity must be 1..2000. Quantity=0 → exception 03. */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_coils_excess_quantity_fails)
{
	/* Spec sec 6.1: max quantity is 2000 (0x07D0). Quantity=2001 → exception 03. */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x07u, 0xD1u}; /* qty=2001 */
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_coils_nonexistent_address_fails)
{
	/* Spec sec 6.1 state diagram: starting address not found → exception 02. */
	uint8_t coil_byte = 0x01u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0x0010u, .access=MCACC_R_PTR, .read={.ptr=&coil_byte, .ix=0u}}
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x01u}; /* addr 0 missing */
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST(mbpdu_read_coils_exception_fc_has_error_flag)
{
	/*
	 * Spec sec 7: "In an exception response, the server sets the MSB of the
	 * function code to 1." → response FC = request FC + 0x80.
	 */
	struct mbinst_s inst = {0};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_COILS, 0x00u, 0x00u, 0x00u, 0x00u}; /* qty=0 */
	uint8_t res[MBPDU_SIZE_MAX];
	mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ((uint8_t)(MBFC_READ_COILS | MB_ERR_FLG), res[0]);
}

/* =========================================================================
 * FC 02 Read Discrete Inputs
 * Spec sec 6.2: Same packing rules as FC 01. Quantity range 1..2000.
 * ========================================================================= */

TEST(mbpdu_read_disc_inputs_single_on)
{
	uint8_t input_byte = 0x01u;
	const struct mbcoil_desc_s inputs[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&input_byte, .ix=0u}}
	};
	struct mbinst_s inst = {.disc_inputs=inputs, .n_disc_inputs=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_DISC_INPUTS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_READ_DISC_INPUTS, res[0]);
	ASSERT_EQ(3u, res_size);
	ASSERT_EQ(1u, res[1]);
	ASSERT_EQ(0x01u, res[2]);
}

TEST(mbpdu_read_disc_inputs_single_off)
{
	uint8_t input_byte = 0x00u;
	const struct mbcoil_desc_s inputs[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&input_byte, .ix=0u}}
	};
	struct mbinst_s inst = {.disc_inputs=inputs, .n_disc_inputs=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_DISC_INPUTS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(3u, res_size);
	ASSERT_EQ(1u, res[1]);
	ASSERT_EQ(0x00u, res[2]);
}

TEST(mbpdu_read_disc_inputs_lsb_first_packing)
{
	/*
	 * Spec sec 6.2: same bit packing as FC 01.
	 * Eight inputs at addresses 0-7. bits 0,2,4,6 set → 0x55.
	 */
	uint8_t byte = 0x55u;
	const struct mbcoil_desc_s inputs[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=0u}},
		{.address=1u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=1u}},
		{.address=2u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=2u}},
		{.address=3u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=3u}},
		{.address=4u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=4u}},
		{.address=5u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=5u}},
		{.address=6u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=6u}},
		{.address=7u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=7u}},
	};
	struct mbinst_s inst = {.disc_inputs=inputs, .n_disc_inputs=8u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_DISC_INPUTS, 0x00u, 0x00u, 0x00u, 0x08u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(3u, res_size);
	ASSERT_EQ(1u, res[1]);
	ASSERT_EQ(0x55u, res[2]);
}

TEST(mbpdu_read_disc_inputs_partial_byte_zero_padded)
{
	/* Same padding rule as FC 01: non-multiple-of-8 quantity → high bits zeroed. */
	uint8_t byte = 0xFFu;
	const struct mbcoil_desc_s inputs[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=0u}},
		{.address=1u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=1u}},
		{.address=2u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=2u}},
		{.address=3u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=3u}},
		{.address=4u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=4u}},
	};
	struct mbinst_s inst = {.disc_inputs=inputs, .n_disc_inputs=5u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_DISC_INPUTS, 0x00u, 0x00u, 0x00u, 0x05u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(3u, res_size);
	ASSERT_EQ(1u, res[1]);
	ASSERT_EQ(0x1Fu, res[2]); /* bits 0-4 set; bits 5-7 padded with zero */
}

TEST(mbpdu_read_disc_inputs_zero_quantity_fails)
{
	/* Spec sec 6.2: quantity 0 → exception 03. */
	uint8_t byte = 0x00u;
	const struct mbcoil_desc_s inputs[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=0u}}
	};
	struct mbinst_s inst = {.disc_inputs=inputs, .n_disc_inputs=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_DISC_INPUTS, 0x00u, 0x00u, 0x00u, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_disc_inputs_excess_quantity_fails)
{
	/* Spec sec 6.2: max quantity 2000 (0x07D0). Quantity=2001 → exception 03. */
	uint8_t byte = 0x00u;
	const struct mbcoil_desc_s inputs[] = {
		{.address=0u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=0u}}
	};
	struct mbinst_s inst = {.disc_inputs=inputs, .n_disc_inputs=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_DISC_INPUTS, 0x00u, 0x00u, 0x07u, 0xD1u}; /* qty=2001 */
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_read_disc_inputs_nonexistent_address_fails)
{
	/* Starting address not found → exception 02. */
	uint8_t byte = 0x01u;
	const struct mbcoil_desc_s inputs[] = {
		{.address=0x0020u, .access=MCACC_R_PTR, .read={.ptr=&byte, .ix=0u}}
	};
	struct mbinst_s inst = {.disc_inputs=inputs, .n_disc_inputs=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_READ_DISC_INPUTS, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

/* =========================================================================
 * FC 05 Write Single Coil
 * Spec sec 6.5: Value 0xFF00 → ON. Value 0x0000 → OFF.
 * Any other value → exception 03 (ILLEGAL DATA VALUE).
 * Normal response echoes the request.
 * ========================================================================= */

TEST(mbpdu_write_single_coil_on_works)
{
	/*
	 * Spec sec 6.5 example: write coil 173 (addr 0x00AC) ON with value 0xFF00.
	 * Normal response echoes the request (function code + address + value).
	 */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {{
		.address=0x00ACu,
		.access=MCACC_RW_PTR,
		.read={.ptr=&coil_byte, .ix=0u},
		.write={.ptr=&coil_byte, .ix=0u},
	}};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0xACu, 0xFFu, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_WRITE_SINGLE_COIL, res[0]);
	ASSERT_EQ(5u, res_size);   /* echo of request */
	ASSERT_EQ(0x00u, res[1]); /* address H */
	ASSERT_EQ(0xACu, res[2]); /* address L */
	ASSERT_EQ(0xFFu, res[3]); /* value H */
	ASSERT_EQ(0x00u, res[4]); /* value L */
	ASSERT_EQ(1u, coil_byte & 0x01u); /* coil set ON */
}

TEST(mbpdu_write_single_coil_off_works)
{
	/* Spec sec 6.5: value 0x0000 → coil OFF. */
	uint8_t coil_byte = 0x01u; /* start ON */
	const struct mbcoil_desc_s coils[] = {{
		.address=0x0010u,
		.access=MCACC_RW_PTR,
		.read={.ptr=&coil_byte, .ix=0u},
		.write={.ptr=&coil_byte, .ix=0u},
	}};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x10u, 0x00u, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(0x00u, res[1]);
	ASSERT_EQ(0x10u, res[2]);
	ASSERT_EQ(0x00u, res[3]);
	ASSERT_EQ(0x00u, res[4]);
	ASSERT_EQ(0u, coil_byte & 0x01u); /* coil set OFF */
}

TEST(mbpdu_write_single_coil_invalid_value_fails)
{
	/*
	 * Spec sec 6.5: "All other values are illegal and will not affect the output."
	 * Invalid value → exception 03 (ILLEGAL DATA VALUE).
	 * Coil state must remain unchanged.
	 */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {{
		.address=0x0000u,
		.access=MCACC_RW_PTR,
		.read={.ptr=&coil_byte, .ix=0u},
		.write={.ptr=&coil_byte, .ix=0u},
	}};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	/* 0x0100 is not 0xFF00 or 0x0000 */
	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0x01u, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
	ASSERT_EQ(0x00u, coil_byte); /* coil must not be changed */
}

TEST(mbpdu_write_single_coil_value_0x0001_invalid)
{
	/* 0x0001 is not a valid coil value → exception 03. */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {{
		.address=0x0000u,
		.access=MCACC_RW_PTR,
		.read={.ptr=&coil_byte, .ix=0u},
		.write={.ptr=&coil_byte, .ix=0u},
	}};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0x00u, 0x01u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_write_single_coil_nonexistent_address_fails)
{
	/* Spec sec 6.5 state diagram: output address not found → exception 02. */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {{
		.address=0x0010u,
		.access=MCACC_RW_PTR,
		.read={.ptr=&coil_byte, .ix=0u},
		.write={.ptr=&coil_byte, .ix=0u},
	}};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_SINGLE_COIL, 0x00u, 0x00u, 0xFFu, 0x00u}; /* addr 0 missing */
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

/* =========================================================================
 * FC 0F Write Multiple Coils
 * Spec sec 6.11: Quantity range 1..1968 (0x07B0).
 * Byte count must equal ceil(quantity / 8) → exception 03 if mismatch.
 * Logical '1' in bit position → output ON. Logical '0' → output OFF.
 * Normal response: [FC][start_addr_H][start_addr_L][qty_H][qty_L]  (no data).
 * ========================================================================= */

TEST(mbpdu_write_multiple_coils_basic)
{
	/*
	 * Write 8 coils starting at address 0. Data byte 0xCD = 1100 1101 binary.
	 * Bit 0 (LSB) → coil 0 ON, bit 1 → coil 1 OFF, bit 2 → coil 2 ON, etc.
	 * Normal response returns FC + starting address + quantity (no data).
	 */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=0u}, .write={.ptr=&coil_byte,.ix=0u}},
		{.address=1u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=1u}, .write={.ptr=&coil_byte,.ix=1u}},
		{.address=2u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=2u}, .write={.ptr=&coil_byte,.ix=2u}},
		{.address=3u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=3u}, .write={.ptr=&coil_byte,.ix=3u}},
		{.address=4u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=4u}, .write={.ptr=&coil_byte,.ix=4u}},
		{.address=5u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=5u}, .write={.ptr=&coil_byte,.ix=5u}},
		{.address=6u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=6u}, .write={.ptr=&coil_byte,.ix=6u}},
		{.address=7u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=7u}, .write={.ptr=&coil_byte,.ix=7u}},
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=8u};
	mbinst_init(&inst);

	uint8_t pdu[] = {
		MBFC_WRITE_MULTIPLE_COILS,
		0x00u, 0x00u,  /* starting address */
		0x00u, 0x08u,  /* quantity of outputs: 8 */
		0x01u,         /* byte count: 1 */
		0xCDu,         /* outputs value: 1100 1101 */
	};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(MBFC_WRITE_MULTIPLE_COILS, res[0]);
	/* Spec: response = FC + starting address + quantity (no data) */
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(0x00u, res[1]); /* starting address H */
	ASSERT_EQ(0x00u, res[2]); /* starting address L */
	ASSERT_EQ(0x00u, res[3]); /* quantity H */
	ASSERT_EQ(0x08u, res[4]); /* quantity L */
	ASSERT_EQ(0xCDu, coil_byte);
}

TEST(mbpdu_write_multiple_coils_partial_byte)
{
	/*
	 * Spec sec 6.11: write 10 coils using 2 data bytes.
	 * Request data: CD 01 → 1100 1101  0000 0001.
	 * Coil 0 (addr 0) ← bit 0 of byte 0 = 1 (ON).
	 * Coil 9 (addr 9) ← bit 1 of byte 1 = 0 (OFF).
	 * Unused bits in last byte (bits 2-7) must be ignored.
	 */
	uint8_t byte_a = 0x00u; /* coils 0-7 */
	uint8_t byte_b = 0x00u; /* coils 8-9 */
	const struct mbcoil_desc_s coils[] = {
		{.address=0u, .access=MCACC_RW_PTR, .read={.ptr=&byte_a,.ix=0u}, .write={.ptr=&byte_a,.ix=0u}},
		{.address=1u, .access=MCACC_RW_PTR, .read={.ptr=&byte_a,.ix=1u}, .write={.ptr=&byte_a,.ix=1u}},
		{.address=2u, .access=MCACC_RW_PTR, .read={.ptr=&byte_a,.ix=2u}, .write={.ptr=&byte_a,.ix=2u}},
		{.address=3u, .access=MCACC_RW_PTR, .read={.ptr=&byte_a,.ix=3u}, .write={.ptr=&byte_a,.ix=3u}},
		{.address=4u, .access=MCACC_RW_PTR, .read={.ptr=&byte_a,.ix=4u}, .write={.ptr=&byte_a,.ix=4u}},
		{.address=5u, .access=MCACC_RW_PTR, .read={.ptr=&byte_a,.ix=5u}, .write={.ptr=&byte_a,.ix=5u}},
		{.address=6u, .access=MCACC_RW_PTR, .read={.ptr=&byte_a,.ix=6u}, .write={.ptr=&byte_a,.ix=6u}},
		{.address=7u, .access=MCACC_RW_PTR, .read={.ptr=&byte_a,.ix=7u}, .write={.ptr=&byte_a,.ix=7u}},
		{.address=8u, .access=MCACC_RW_PTR, .read={.ptr=&byte_b,.ix=0u}, .write={.ptr=&byte_b,.ix=0u}},
		{.address=9u, .access=MCACC_RW_PTR, .read={.ptr=&byte_b,.ix=1u}, .write={.ptr=&byte_b,.ix=1u}},
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=10u};
	mbinst_init(&inst);

	uint8_t pdu[] = {
		MBFC_WRITE_MULTIPLE_COILS,
		0x00u, 0x00u,  /* starting address */
		0x00u, 0x0Au,  /* quantity: 10 */
		0x02u,         /* byte count: ceil(10/8) = 2 */
		0xCDu,         /* byte 0: 1100 1101 */
		0x01u,         /* byte 1: 0000 0001 (only bits 0-1 used) */
	};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT(!(res[0] & MB_ERR_FLG));
	ASSERT_EQ(5u, res_size);
	ASSERT_EQ(0x00u, res[3]); /* quantity H */
	ASSERT_EQ(0x0Au, res[4]); /* quantity L */
	ASSERT_EQ(0xCDu, byte_a); /* coils 0-7 */
	/* coil 8 = bit 0 of 0x01 = 1 (ON), coil 9 = bit 1 = 0 (OFF) */
	ASSERT_EQ(1u, byte_b & 0x01u); /* coil 8 ON */
	ASSERT_EQ(0u, byte_b & 0x02u); /* coil 9 OFF */
}

TEST(mbpdu_write_multiple_coils_zero_quantity_fails)
{
	/* Spec sec 6.11: quantity must be 1..1968. Quantity=0 → exception 03. */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {{
		.address=0u, .access=MCACC_RW_PTR,
		.read={.ptr=&coil_byte,.ix=0u}, .write={.ptr=&coil_byte,.ix=0u},
	}};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_write_multiple_coils_excess_quantity_fails)
{
	/* Spec sec 6.11: max quantity is 1968 (0x07B0). Quantity=1969 → exception 03. */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {{
		.address=0u, .access=MCACC_RW_PTR,
		.read={.ptr=&coil_byte,.ix=0u}, .write={.ptr=&coil_byte,.ix=0u},
	}};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {
		MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u,
		0x07u, 0xB1u, /* qty=1969, exceeds max of 1968 */
		0xF9u,        /* byte count */
	};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_write_multiple_coils_byte_count_mismatch_fails)
{
	/*
	 * Spec sec 6.11: byte count must equal ceil(quantity / 8).
	 * Quantity=3 requires byte_count=1. Sending byte_count=2 → exception 03.
	 */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {
		{.address=0u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=0u}, .write={.ptr=&coil_byte,.ix=0u}},
		{.address=1u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=1u}, .write={.ptr=&coil_byte,.ix=1u}},
		{.address=2u, .access=MCACC_RW_PTR, .read={.ptr=&coil_byte,.ix=2u}, .write={.ptr=&coil_byte,.ix=2u}},
	};
	struct mbinst_s inst = {.coils=coils, .n_coils=3u};
	mbinst_init(&inst);

	uint8_t pdu[] = {
		MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u,
		0x00u, 0x03u, /* qty=3 → byte_count must be 1 */
		0x02u,        /* byte_count=2 (wrong) */
		0x07u, 0x00u,
	};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_VAL, res[1]);
}

TEST(mbpdu_write_multiple_coils_nonexistent_address_fails)
{
	/* Starting address not found → exception 02. */
	uint8_t coil_byte = 0x00u;
	const struct mbcoil_desc_s coils[] = {{
		.address=0x0010u, .access=MCACC_RW_PTR,
		.read={.ptr=&coil_byte,.ix=0u}, .write={.ptr=&coil_byte,.ix=0u},
	}};
	struct mbinst_s inst = {.coils=coils, .n_coils=1u};
	mbinst_init(&inst);

	uint8_t pdu[] = {
		MBFC_WRITE_MULTIPLE_COILS, 0x00u, 0x00u, /* addr 0 missing */
		0x00u, 0x01u,
		0x01u,
		0xFFu,
	};
	uint8_t res[MBPDU_SIZE_MAX];
	size_t res_size = mbpdu_handle_req(&inst, pdu, sizeof pdu, res);

	ASSERT_EQ(2u, res_size);
	ASSERT(res[0] & MB_ERR_FLG);
	ASSERT_EQ(MB_ILLEGAL_DATA_ADDR, res[1]);
}

TEST_MAIN(
	/* FC 01 Read Coils */
	mbpdu_read_coils_single_on,
	mbpdu_read_coils_single_off,
	mbpdu_read_coils_lsb_first_packing,
	mbpdu_read_coils_partial_byte_zero_padded,
	mbpdu_read_coils_multi_byte_response,
	mbpdu_read_coils_zero_quantity_fails,
	mbpdu_read_coils_excess_quantity_fails,
	mbpdu_read_coils_nonexistent_address_fails,
	mbpdu_read_coils_exception_fc_has_error_flag,
	/* FC 02 Read Discrete Inputs */
	mbpdu_read_disc_inputs_single_on,
	mbpdu_read_disc_inputs_single_off,
	mbpdu_read_disc_inputs_lsb_first_packing,
	mbpdu_read_disc_inputs_partial_byte_zero_padded,
	mbpdu_read_disc_inputs_zero_quantity_fails,
	mbpdu_read_disc_inputs_excess_quantity_fails,
	mbpdu_read_disc_inputs_nonexistent_address_fails,
	/* FC 05 Write Single Coil */
	mbpdu_write_single_coil_on_works,
	mbpdu_write_single_coil_off_works,
	mbpdu_write_single_coil_invalid_value_fails,
	mbpdu_write_single_coil_value_0x0001_invalid,
	mbpdu_write_single_coil_nonexistent_address_fails,
	/* FC 0F Write Multiple Coils */
	mbpdu_write_multiple_coils_basic,
	mbpdu_write_multiple_coils_partial_byte,
	mbpdu_write_multiple_coils_zero_quantity_fails,
	mbpdu_write_multiple_coils_excess_quantity_fails,
	mbpdu_write_multiple_coils_byte_count_mismatch_fails,
	mbpdu_write_multiple_coils_nonexistent_address_fails
);
