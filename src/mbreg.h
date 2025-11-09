/**
 * @file mbreg.h
 * @brief Modbus Register Descriptor - Structure and functions for register handling
 * @author Jonas Almås
 *
 * @details This module defines the register descriptor structure and associated
 * functions for managing Modbus registers (16-bit values and multi-word data types).
 * Supports multiple access methods including direct memory access, function callbacks,
 * constant values, and block/array operations with optional runtime locking.
 *
 * @see mbinst.h for register map integration
 *
 * MISRA Deviations:
 * - Rule 19.2: The union keyword should not be used
 *   Rational: Necessary to describe the protocol
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

#ifndef MBREG_H_INCLUDED
#define MBREG_H_INCLUDED

#include "mbdef.h"
#include <stddef.h>
#include <stdint.h>

enum {
	MRTYPE_UNSIGNED = 1u<<0,
	MRTYPE_SIGNED = 1u<<1,
	MRTYPE_FLOAT = 1u<<2,

	MRTYPE_SIZE_8 = 1u<<3/*8*/,
	MRTYPE_SIZE_16 = 1u<<4/*16*/,
	MRTYPE_SIZE_32 = 1u<<5/*32*/,
	MRTYPE_SIZE_64 = 1u<<6/*64*/,
	MRTYPE_SIZE_MAX = MRTYPE_SIZE_64,

	MRTYPE_MASK = (1u<<7)-1u,
	MRTYPE_SIZE_MASK = MRTYPE_MASK & ~7u,
};

/**
 * @brief Modbus register data type
 */
enum mbreg_type_e {
	/**
	 * @brief Block register flag for array access
	 *
	 * When combined with other types (e.g., MRTYPE_U16 | MRTYPE_BLOCK),
	 * enables array-style access where consecutive addresses map to
	 * consecutive array elements.
	 *
	 * @note 7-bit values are handled as one per 16-bit register
	 * @note Use n_block_entries to specify array size
	 */
	MRTYPE_BLOCK = 512u,

	MRTYPE_U8 = MRTYPE_SIZE_8 | MRTYPE_UNSIGNED, /**< padded to 16-bit for protocol */
	MRTYPE_U16 = MRTYPE_SIZE_16 | MRTYPE_UNSIGNED,
	MRTYPE_U32 = MRTYPE_SIZE_32 | MRTYPE_UNSIGNED,
	MRTYPE_U64 = MRTYPE_SIZE_64 | MRTYPE_UNSIGNED,

	MRTYPE_I8 = MRTYPE_SIZE_8 | MRTYPE_SIGNED, /**< padded to 16-bit for protocol */
	MRTYPE_I16 = MRTYPE_SIZE_16 | MRTYPE_SIGNED,
	MRTYPE_I32 = MRTYPE_SIZE_32 | MRTYPE_SIGNED,
	MRTYPE_I64 = MRTYPE_SIZE_64 | MRTYPE_SIGNED,

	MRTYPE_F32 = MRTYPE_SIZE_32 | MRTYPE_FLOAT,
	MRTYPE_F64 = MRTYPE_SIZE_64 | MRTYPE_FLOAT,
};

/**
 * @brief Modbus register access method
 *
 * Defines how a register can be accessed (read/write methods)
 */
enum mbreg_access_e {
	MRACC_R_VAL = 1u<<0, /**< Read from constant value stored in descriptor */

	MRACC_R_PTR = 1u<<1, /**< Read from pointer */
	MRACC_W_PTR = 1u<<2, /**< Write to pointer */
	MRACC_RW_PTR = MRACC_R_PTR | MRACC_W_PTR, /**< Read/write via pointer */

	MRACC_R_FN = 1u<<3, /**< Read via function callback */
	MRACC_W_FN = 1u<<4, /**< Write via function callback */
	MRACC_RW_FN = MRACC_R_FN | MRACC_W_FN, /**< Read/write via function callbacks */

	MRACC_R_MASK = MRACC_R_VAL | MRACC_R_PTR | MRACC_R_FN, /**< Mask for read access methods */
	MRACC_W_MASK = MRACC_W_PTR | MRACC_W_FN, /**< Mask for write access methods */
};

/**
 * @brief Modbus Register Descriptor
 *
 * Describes a single Modbus register or block of registers, including its address,
 * data type, access methods, and optional callbacks. This structure defines how
 * the Modbus library should handle read/write operations for a specific register.
 *
 * @note All registers in an array must be sorted by address in ascending order
 */
struct mbreg_desc_s {
	/**
	 * @brief Modbus register address
	 *
	 * The starting address for this register.
	 * For multi-word registers, this is the address of the first (most significant) word.
	 *
	 * @note Valid range: [0x0000 - 0x270E] (Standard Modbus register range)
	 * @note For block registers, consecutive addresses map to array elements
	 */
	uint16_t address;

	/**
	 * @brief Register data type and block flag
	 *
	 * Specifies the data type (U8, U16, U32, U64, I8, I16, I32, I64, F32, F64)
	 * and optionally the MRTYPE_BLOCK flag for array access.
	 */
	enum mbreg_type_e type;

	/**
	 * @brief Access method configuration
	 *
	 * Specifies how this register can be accessed: constant value, memory pointer,
	 * or function callbacks. Combine read and write methods as needed.
	 *
	 * @note Must specify only one read method for readable registers
	 * @note Must specify only one write method for writable registers
	 */
	enum mbreg_access_e access;

	/**
	 * @brief Read access configuration
	 *
	 * Tagged union containing the data source for read operations.
	 * The active member depends on the access method specified in the access field.
	 *
	 * @note For MRACC_R_VAL: Use value members (u8, u16, u32, etc.)
	 * @note For MRACC_R_PTR: Use pointer members (pu8, pu16, pu32, etc.)
	 * @note For MRACC_R_FN: Use function members (fu8, fu16, fu32, etc.)
	 */
	union {
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		float f32;
		double f64;
		volatile uint8_t *pu8;
		volatile uint16_t *pu16;
		volatile uint32_t *pu32;
		volatile uint64_t *pu64;
		volatile int8_t *pi8;
		volatile int16_t *pi16;
		volatile int32_t *pi32;
		volatile int64_t *pi64;
		volatile float *pf32;
		volatile double *pf64;
		uint8_t (*fu8)(void);
		uint16_t (*fu16)(void);
		uint32_t (*fu32)(void);
		uint64_t (*fu64)(void);
		int8_t (*fi8)(void);
		int16_t (*fi16)(void);
		int32_t (*fi32)(void);
		int64_t (*fi64)(void);
		float (*ff32)(void);
		double (*ff64)(void);
	} read;

	/**
	 * @brief Write access configuration
	 *
	 * Tagged union containing the data destination for write operations.
	 * The active member depends on the access method specified in the access field.
	 *
	 * @note For MRACC_W_PTR: Use pointer members (pu8, pu16, pu32, etc.)
	 * @note For MRACC_W_FN: Use function members (fu8, fu16, fu32, etc.)
	 */
	union {
		volatile uint8_t *pu8;
		volatile uint16_t *pu16;
		volatile uint32_t *pu32;
		volatile uint64_t *pu64;
		volatile int8_t *pi8;
		volatile int16_t *pi16;
		volatile int32_t *pi32;
		volatile int64_t *pi64;
		volatile float *pf32;
		volatile double *pf64;
		enum mbstatus_e (*fu8)(uint8_t);
		enum mbstatus_e (*fu16)(uint16_t);
		enum mbstatus_e (*fu32)(uint32_t);
		enum mbstatus_e (*fu64)(uint64_t);
		enum mbstatus_e (*fi8)(int8_t);
		enum mbstatus_e (*fi16)(int16_t);
		enum mbstatus_e (*fi32)(int32_t);
		enum mbstatus_e (*fi64)(int64_t);
		enum mbstatus_e (*ff32)(float);
		enum mbstatus_e (*ff64)(double);
	} write;

	/**
	 * @brief Dynamic read lock callback
	 *
	 * Optional callback to check if register reads are currently locked.
	 * Called before every read operation to enable runtime access control.
	 *
	 * @retval 1 Read locked (will return MB_ILLEGAL_DATA_ADDR)
	 * @retval 0 Read allowed
	 *
	 * @note If NULL, reads are always allowed
	 * @note Useful for implementing time-based locks, security modes, etc.
	 */
	int (*rlock_cb)(void);

	/**
	 * @brief Dynamic write lock callback
	 *
	 * Optional callback to check if register writes are currently locked.
	 * Called before every write operation to enable runtime access control.
	 *
	 * @retval 1 Write locked (will return MB_ILLEGAL_DATA_ADDR)
	 * @retval 0 Write allowed
	 *
	 * @note If NULL, writes are always allowed
	 * @note Useful for implementing safety interlocks, calibration modes, etc.
	 */
	int (*wlock_cb)(void);

	/**
	 * @brief Write lock override callback
	 *
	 * Optional callback to override write restrictions based on context.
	 * Called when wlock_cb returns 1, allowing conditional override.
	 *
	 * @param reg Pointer to this register descriptor
	 * @param reg_start_addr Starting address of the write operation
	 * @param n_remaining_regs Number of remaining registers in the write operation
	 * @param val Pointer to the data being written
	 *
	 * @retval 0 Maintain write lock (reject write)
	 * @retval 1 Override lock (allow write)
	 *
	 * @note If NULL, locked writes are always rejected
	 * @note Useful for implementing privileged access
	 */
	int (*wlock_override_cb)(
		const struct mbreg_desc_s *reg,
		uint16_t reg_start_addr,
		size_t n_remaining_regs,
		const uint8_t *val);

	/**
	 * @brief Number of elements in block registers
	 *
	 * For registers with MRTYPE_BLOCK flag, specifies the number of array elements.
	 * Each element occupies addresses equal to the base type size.
	 *
	 * @note Only used when MRTYPE_BLOCK flag is set
	 * @note Example: MRTYPE_U16 | MRTYPE_BLOCK with n_block_entries=10 occupies 10 addresses
	 * @note Example: MRTYPE_U32 | MRTYPE_BLOCK with n_block_entries=5 occupies 10 addresses (5*2)
	 * @note Must be > 0 for block registers
	 */
	size_t n_block_entries;

	/**
	 * @brief Post-write callback
	 *
	 * Optional callback executed after a successful write operation.
	 * Called after the data has been written but before the response is sent.
	 *
	 * @note If NULL, no post-write action is taken
	 * @note Called for each register write, even in multi-register operations
	 * @note Useful for triggering hardware updates, calculations, etc.
	 * @note Should not perform time-consuming operations
	 */
	void (*post_write_cb)(void);
};

/** @brief Only for internal use */
#define MBREG_READ_NO_ACCESS 0u /* No read access method present */
#define MBREG_READ_LOCKED (SIZE_MAX-1u) /* Register is locked */
#define MBREG_READ_DEV_FAIL SIZE_MAX /* Invalid coil descriptor configuration */

/**
 * @brief Get size of Modbus register in bytes
 *
 * Calculates the protocol size of a register based on its type.
 * 8-bit registers are reported as 16-bit since Modbus protocol uses 16-bit words.
 *
 * @param reg Pointer to the register descriptor
 *
 * @return Size in bytes (2, 4, or 8), or 0 for invalid types
 *
 * @note For MRTYPE_BLOCK registers, this returns the size of one element
 * @note U8/I8 types return 2 bytes (padded to 16-bit for protocol compliance)
 */
extern size_t mbreg_size(const struct mbreg_desc_s *reg);

/**
 * @brief Finds a Modbus register descriptor by address
 *
 * Searches for a register that contains the specified address. Uses binary search
 * for efficient lookup in large register maps. Handles block registers by checking
 * if the address falls within the block's address range.
 *
 * @param regs Array of register descriptors (must be sorted in ascending address order)
 * @param n_regs Number of entries in the regs array
 * @param addr Modbus address to search for
 *
 * @return Pointer to the register descriptor containing the address, or NULL if not found
 *
 * @note Time complexity: O(log n) due to binary search. O(n) for smaller sets
 * @note For block registers, any address within the block range will return the block descriptor
 * @note Assumes register array is sorted by address in ascending order
 */
extern const struct mbreg_desc_s *mbreg_find_desc(
	const struct mbreg_desc_s *regs,
	size_t n_regs,
	uint16_t addr);

/**
 * @brief Read the value of a Modbus register
 *
 * Reads data from a register using the configured access method (pointer, function, or constant value).
 * Handles partial reads for multi-word registers and applies word order swapping if requested.
 *
 * @param reg Pointer to the register descriptor
 * @param addr Starting address to read from (can be sub-address within a multi-word register)
 * @param n_remaining_regs Maximum number of 16-bit words to read
 * @param res Pointer to buffer where read data will be stored (big-endian). If NULL, dry run (Check read allowed)
 * @param swap_words If non-zero, swaps word order for multi-word values (U32, U64, F32, F64)
 *
 * @retval Number of 16-bit words actually read
 * @retval MBREG_READ_NO_ACCESS (0) No read access method (Ignored when reading)
 * @retval MBREG_READ_LOCKED (SIZE_MAX-1) Register read locked
 * @retval MBREG_READ_DEV_FAIL (SIZE_MAX) Device fault
 *
 * @note Data is returned in big-endian format as per Modbus specification
 * @note For block registers, calculates the correct array index automatically
 * @note Word order swapping should only be used for input registers and is non-standard
 */
extern size_t mbreg_read(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	size_t n_remaining_regs,
	uint8_t *res,
	int swap_words);

/**
 * @brief Check if a Modbus register can be written to
 *
 * Validates write permissions by checking access flags, lock callbacks, and write function availability.
 * This function should be called before attempting to write to ensure the operation is allowed.
 *
 * @param reg Pointer to the register descriptor
 * @param addr Address to write to (can be sub-address within a multi-word register)
 * @param start_addr Starting address of the write operation (for context to callbacks)
 * @param n_remaining_regs Number of remaining 16-bit words in the write operation
 * @param val Pointer to the data being written (for validation by callbacks)
 *
 * @retval n-regs to write if write is allowed
 * @retval 0 if write should be rejected
 *
 * @note Checks register access flags (MRACC_W_PTR or MRACC_W_FN)
 * @note Calls wlock_cb if defined to check dynamic write locks
 * @note Calls wlock_override_cb if defined to allow override of lock conditions
 */
extern size_t mbreg_write_allowed(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	uint16_t start_addr,
	size_t n_remaining_regs,
	const uint8_t *val);

/**
 * @brief Write to a Modbus register
 *
 * Performs the actual write operation to a register using the configured access method.
 * Handles partial writes for multi-word registers and proper endian conversion.
 *
 * @param reg Pointer to the register descriptor
 * @param addr Address to write to (can be sub-address within a multi-word register)
 * @param n_remaining_regs Maximum number of 16-bit words to write
 * @param val Pointer to write data (big-endian format as per Modbus specification)
 * @param n_written Pointer to out parameter reporting number of 16-bit words actually written (Can be set to for failing writes)
 *
 * @return Modbus status code
 *
 * @warning This function does not check write permissions - call mbreg_write_allowed() first
 * @note Input data must be in big-endian format as received from Modbus protocol
 * @note For function-based writes, the write function return value determines success/failure
 * @note For block registers, calculates the correct array index automatically
 * @note Handles endian conversion and partial register updates correctly
 */
extern enum mbstatus_e mbreg_write(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	size_t n_remaining_regs,
	const uint8_t *val,
	size_t *n_written);

/**
 * @brief Write masked Modbus register
 *
 * @param reg Pointer to the register descriptor
 * @param addr Address to write to (can be sub-address within a multi-word register)
 * @param and_mask
 * @param or_mask
 *
 * @return Modbus status code
 *
 * @warning This function does not check write permissions - call mbreg_write_allowed() first
 */
extern enum mbstatus_e mbreg_mask_write(
	const struct mbreg_desc_s *reg,
	uint16_t addr,
	uint16_t and_mask,
	uint16_t or_mask);

#endif /* MBREG_H_INCLUDED */
