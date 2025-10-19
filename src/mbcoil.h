/**
 * @file mbcoil.h
 * @brief Modbus Coil Descriptor - Structure and functions for coil handling
 * @author Jonas Almås
 *
 * @details This module defines the coil descriptor structure and associated
 * functions for managing Modbus coils (single-bit values). Supports multiple
 * access methods including direct memory access, function callbacks, and
 * constant values with optional runtime locking.
 *
 * @see mbinst.h for coil map integration
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

#ifndef MBCOIL_H_INCLUDED
#define MBCOIL_H_INCLUDED

#include "mbdef.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Modbus coil access method
 *
 * Defines how a coil can be accessed (read/write methods).
 * Multiple access methods can be combined using bitwise OR.
 */
enum mbcoil_access_e {
	MCACC_R_VAL = 1<<0, /**< Read from constant value stored in descriptor */

	MCACC_R_PTR = 1<<1, /**< Read from memory pointer using bit index */
	MCACC_W_PTR = 1<<2, /**< Write to memory pointer using bit index */
	MCACC_RW_PTR = MCACC_R_PTR | MCACC_W_PTR, /**< Read/write via memory pointer */

	MCACC_R_FN = 1<<3, /**< Read via function callback */
	MCACC_W_FN = 1<<4, /**< Write via function callback */
	MCACC_RW_FN = MCACC_R_FN | MCACC_W_FN, /**< Read/write via function callbacks */

	MCACC_R_MASK = MCACC_R_VAL | MCACC_R_PTR | MCACC_R_FN, /**< Mask for read access methods */
	MCACC_W_MASK = MCACC_W_PTR | MCACC_W_FN, /**< Mask for write access methods */
};

/**
 * @brief Modbus Coil Descriptor
 *
 * Describes a single Modbus coil (single-bit value), including its address,
 * access methods, and optional callbacks. This structure defines how the
 * Modbus library should handle read/write operations for a specific coil.
 *
 * @note All coils in an array must be sorted by address in ascending order
 * @note Coils are single-bit values (0 = OFF, 1 = ON)
 * @note Multiple coils can share the same byte using different bit indices
 */
struct mbcoil_desc_s {
	/**
	 * @brief Modbus coil address
	 *
	 * The address for this coil.
	 * Each coil occupies exactly one address regardless of the underlying storage.
	 *
	 * @note Valid range: [0x0000 - 0x270E] (Standard Modbus coil range)
	 * @note Multiple coils can map to different bits of the same byte
	 */
	uint16_t address;

	/**
	 * @brief Access method configuration
	 *
	 * Specifies how this coil can be accessed: constant value, memory pointer
	 * with bit index, or function callbacks. Combine read and write methods as needed.
	 *
	 * @note Must specify only one read method for readable coils
	 * @note Must specify only one write method for writable coils
	 * @note Discrete inputs only use read-only access methods
	 */
	enum mbcoil_access_e access;

	/**
	 * @brief Read access configuration
	 *
	 * Tagged union containing the data source for read operations.
	 * The active member depends on the access method specified in the access field.
	 *
	 * @note For MCACC_R_VAL: Use val member (0 or 1, normalized automatically)
	 * @note For MCACC_R_PTR: Use ptr and ix members for bit-level access
	 * @note For MCACC_R_FN: Use fn member (function returning 0 or 1)
	 */
	union {
		/**
		 * @brief Constant coil value
		 *
		 * Static value for read-only coils. Any non-zero value is normalized to 1.
		 *
		 * @note Used with MCACC_R_VAL access method
		 * @note Automatically normalized to 0 or 1
		 */
		uint8_t val;

		/**
		 * @brief Pointer-based access with bit index
		 *
		 * Allows reading/writing individual bits within a byte.
		 * Multiple coils can share the same byte using different bit indices.
		 */
		struct {
			volatile uint8_t *ptr; /**< Pointer to byte containing the coil bit */
			uint8_t ix; /**< Bit index within the byte [0-7], where 0 = LSB */
		};

		/**
		 * @brief Read function callback
		 *
		 * Function called to read the coil state. Should return 0 for OFF, 1 for ON.
		 *
		 * @note Used with MCACC_R_FN access method
		 * @note Return value is automatically normalized to 0 or 1
		 * @note Should be fast and non-blocking
		 */
		int (*fn)(void);
	} read;

	/**
	 * @brief Write access configuration
	 *
	 * Tagged union containing the data destination for write operations.
	 * The active member depends on the access method specified in the access field.
	 *
	 * @note For MCACC_W_PTR: Use ptr and ix members for bit-level access
	 * @note For MCACC_W_FN: Use fn member (function accepting 0 or 1)
	 */
	union {
		/**
		 * @brief Pointer-based write access with bit index
		 *
		 * Allows writing individual bits within a byte.
		 * The specified bit is set to 0 or 1 while preserving other bits.
		 */
		struct {
			volatile uint8_t *ptr; /**< Pointer to byte containing the coil bit */
			uint8_t ix; /**< Bit index within the byte [0-7], where 0 = LSB */
		};

		/**
		 * @brief Write function callback
		 *
		 * Function called to write the coil state. Receives 0 for OFF, 1 for ON.
		 *
		 * @param value New coil value (0 = OFF, 1 = ON)
		 *
		 * @return Modbus status code
		 *
		 * @note Used with MCACC_W_FN access method
		 * @note Should be fast and non-blocking
		 * @note Return value determines if the write operation succeeds
		 */
		enum mbstatus_e (*fn)(int value);
	} write;

	/**
	 * @brief Dynamic read lock callback
	 *
	 * Optional callback to check if coil reads are currently locked.
	 * Called before every read operation to enable runtime access control.
	 *
	 * @retval 1 Read locked (will return MB_ILLEGAL_DATA_ADDR)
	 * @retval 0 Read allowed
	 *
	 * @note If NULL, reads are always allowed
	 * @note Useful for implementing security modes, operational states, etc.
	 */
	int (*rlock_cb)(void);

	/**
	 * @brief Dynamic write lock callback
	 *
	 * Optional callback to check if coil writes are currently locked.
	 * Called before every write operation to enable runtime access control.
	 *
	 * @retval 1 Write locked (will return MB_ILLEGAL_DATA_ADDR)
	 * @retval 0 Write allowed
	 *
	 * @note If NULL, writes are always allowed
	 * @note Useful for implementing safety interlocks, operational modes, etc.
	 */
	int (*wlock_cb)(void);

	/**
	 * @brief Post-write callback
	 *
	 * Optional callback executed after a successful write operation.
	 * Called after the coil state has been updated but before the response is sent.
	 *
	 * @note If NULL, no post-write action is taken
	 * @note Called for each coil write, even in multi-coil operations
	 * @note Useful for triggering hardware updates, state changes, etc.
	 * @note Should not perform time-consuming operations
	 */
	void (*post_write_cb)(void);
};

/** @brief Only for internal use */
enum {
	MBCOIL_READ_OFF=0,
	MBCOIL_READ_ON=1,
	MBCOIL_READ_LOCKED, /* Coil is locked */
	MBCOIL_READ_NO_ACCESS, /* No read access method present */
	MBCOIL_READ_DEV_FAIL, /* Invalid coil descriptor configuration */
};

/**
 * @brief Find coil descriptor by address
 *
 * Searches for a coil descriptor that matches the specified address using binary search
 * for efficient lookup in large coil maps.
 *
 * @param coils Array of coil descriptors (must be sorted in ascending address order)
 * @param n_coils Number of entries in the coils array
 * @param addr Modbus coil address to search for
 *
 * @return Pointer to the coil descriptor if found, NULL if no match
 *
 * @note Time complexity: O(log n) due to binary search. O(n) for smaller sets
 * @note Assumes coil array is sorted by address in ascending order
 */
extern const struct mbcoil_desc_s *mbcoil_find_desc(
	const struct mbcoil_desc_s *coils,
	size_t n_coils,
	uint16_t addr);

/**
 * @brief Read a coil value
 *
 * Reads the current state of a coil using the configured access method
 * (pointer with bit index, function callback, or constant value).
 *
 * @param coil Pointer to the coil descriptor
 *
 * @retval MBCOIL_READ_OFF (0)
 * @retval MBCOIL_READ_ON (1)
 * @retval MBCOIL_READ_LOCKED Coil is locked
 * @retval MBCOIL_READ_NO_ACCESS No read access method present
 * @retval MBCOIL_READ_DEV_FAIL Invalid coil descriptor configuration
 *
 * @note For pointer access, uses the bit index to extract the correct bit
 * @note For function access, calls the read function and returns its result (Normalized)
 * @note For value access, returns the stored constant value (Normalized)
 */
extern int mbcoil_read(const struct mbcoil_desc_s *coil);

/**
 * @brief Check if writing to this coil is allowed
 *
 * Validates write permissions by checking access flags and lock callbacks.
 * This function should be called before attempting to write to ensure the
 * operation is allowed.
 *
 * @param coil Pointer to the coil descriptor
 *
 * @retval 1 Allowed
 * @retval 0 write should be rejected
 *
 * @note Checks coil access flags (MCACC_W_PTR or MCACC_W_FN)
 * @note Calls wlock_cb if defined to check dynamic write locks
 */
extern int mbcoil_write_allowed(const struct mbcoil_desc_s *coil);

/**
 * @brief Write a coil value
 *
 * Performs the actual write operation to a coil using the configured access method.
 * Updates the bit in memory (for pointer access) or calls the write function.
 *
 * @param coil Pointer to the coil descriptor
 * @param value New coil value (0 = OFF, non-zero = ON)
 *
 * @return Modbus status code
 *
 * @warning This function does not check write permissions - call mbcoil_write_allowed() first
 * @note For pointer access, modifies the specific bit using the configured bit index
 * @note For function access, calls the write function and returns its result
 */
extern enum mbstatus_e mbcoil_write(const struct mbcoil_desc_s *coil, uint8_t value);

#endif /* MBCOIL_H_INCLUDED */
