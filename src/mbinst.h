/**
 * @file mbinst.h
 * @brief Modbus Instance - Core slave instance structure and configuration
 * @author Jonas Almås
 *
 * @details This module defines the main Modbus slave instance structure that
 * holds all configuration and data mappings for a Modbus slave device. The
 * instance is transport-agnostic and can be used with serial (RTU) or
 * Ethernet (TCP) communication.
 *
 * @see mbcoil.h for coil descriptor structure
 * @see mbreg.h for register descriptor structure
 * @see mbpdu.h for protocol data unit handling
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

#ifndef MBINST_H_INCLUDED
#define MBINST_H_INCLUDED

#include "mbdef.h"
#include "mbcoil.h"
#include "mbfile.h"
#include "mbpdu.h"
#include "mbreg.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Modbus slave instance configuration and data mappings
 *
 * This structure contains all the configuration and data mappings for a Modbus slave device.
 *
 * @note The instance is stateless and thread-safe - multiple instances can coexist
 * @note All descriptor arrays must be sorted in ascending address order
 * @note NULL pointers are allowed for unused data types
 */
struct mbinst_s {
	/**
	 * @brief Discrete input descriptor map (Read-only single bit values)
	 *
	 * Maps Modbus discrete input addresses to data sources. Discrete inputs are
	 * read-only bits typically used for status indicators, sensor states, etc.
	 * Accessed via Modbus function code 0x02 (Read Discrete Inputs).
	 *
	 * @note Can be left as NULL if no discrete inputs are needed
	 * @note Must be sorted in ascending address order for binary search
	 */
	const struct mbcoil_desc_s *disc_inputs;
	size_t n_disc_inputs; /**< Number of discrete input descriptors */

	/**
	 * @brief Coil descriptor map (Read/write single bit values)
	 *
	 * Maps Modbus coil addresses to data sources and sinks. Coils are read/write
	 * bits typically used for control outputs, relay states, digital I/O, etc.
	 * Accessed via function codes 0x01 (Read), 0x05 (Write Single), 0x0F (Write Multiple).
	 *
	 * @note Can be left as NULL if no coils are needed
	 * @note Must be sorted in ascending address order for binary search
	 */
	const struct mbcoil_desc_s *coils;
	size_t n_coils; /**< Number of coil descriptors */

	/**
	 * @brief Input register descriptor map (Read-only 16-bit values)
	 *
	 * Maps Modbus input register addresses to data sources. Input registers are
	 * read-only 16-bit values typically used for sensor readings, measurements, status values.
	 * Accessed via Modbus function code 0x04 (Read Input Registers).
	 * Supports multi-word data types (U32, U64, F32, F64).
	 *
	 * @note Can be left as NULL if no input registers are needed
	 * @note Must be sorted in ascending address order for binary search
	 * @note Word order swapping (swap_words) only affects input registers
	 */
	const struct mbreg_desc_s *input_regs;
	size_t n_input_regs; /**< Number of input register descriptors */

	/**
	 * @brief Holding register descriptor map (Read/write 16-bit values)
	 *
	 * Maps Modbus holding register addresses to data sources and sinks. Holding registers
	 * are read/write 16-bit values typically used for configuration, setpoints, control values.
	 * Accessed via function codes 0x03 (Read), 0x06 (Write Single), 0x10 (Write Multiple).
	 * Supports multi-word data types and block/array access.
	 *
	 * @note Can be left as NULL if no holding registers are needed
	 * @note Must be sorted in ascending address order for binary search
	 */
	const struct mbreg_desc_s *hold_regs;
	size_t n_hold_regs; /**< Number of holding register descriptors */

	/* TODO */
	const struct mbfile_desc_s *files;
	size_t n_files;

	/**
	 * @brief Custom function handler for unsupported or missing function codes
	 *
	 * Called when a Modbus function code is not handled by the standard handlers
	 * or when coil/register maps are NULL. Allows implementation of custom functions,
	 * vendor-specific extensions, or alternative handling of standard functions.
	 *
	 * @param inst Pointer to this Modbus instance (for accessing configuration)
	 * @param fc Modbus function code that was not handled
	 * @param data Pointer to PDU data (excluding the function code byte)
	 * @param len Length of PDU data in bytes
	 * @param res Pointer to response PDU structure to populate
	 *
	 * @retval MB_OK Success
	 * @retval mbstatus_e Error codes for failure
	 *
	 * @note Can be left as NULL if custom functions are not needed
	 * @note Must populate res->data, and res->data_size on success
	 * @note Should return appropriate error codes for invalid requests
	 */
	enum mbstatus_e (*handle_fn_cb)(
		const struct mbinst_s *inst,
		const uint8_t *req,
		size_t req_len,
		struct mbpdu_buf_s *res);

	/**
	 * @brief Commit callback for coil write operations
	 *
	 * Called once after all coil write operations in a request complete successfully.
	 * Useful for implementing data persistence (e.g., saving to flash memory),
	 * applying changes to hardware, or triggering side effects.
	 *
	 * @param inst Pointer to this Modbus instance
	 *
	 * @note Can be left as NULL if commit functionality is not needed
	 * @note Called only once per request, even if multiple coils are written
	 * @note Called after all individual post_write_cb callbacks have executed
	 * @note Not called if any coil write operation fails
	 */
	void (*commit_coils_write_cb)(const struct mbinst_s *inst);

	/**
	 * @brief Commit callback for register write operations
	 *
	 * Called once after all register write operations in a request complete successfully.
	 * Useful for implementing data persistence (e.g., saving to flash memory),
	 * applying configuration changes, or triggering computational updates.
	 *
	 * @param inst Pointer to this Modbus instance
	 *
	 * @note Can be left as NULL if commit functionality is not needed
	 * @note Called only once per request, even if multiple registers are written
	 * @note Called after all individual post_write_cb callbacks have executed
	 * @note Not called if any register write operation fails
	 */
	void (*commit_regs_write_cb)(const struct mbinst_s *inst);

	/**
	 * @brief Allow extended file record numbers
	 *
	 * Remove limit of 10'000 file records.
	 *
	 * @note Value is a u16
	 */
	int allow_ext_file_recs;

	/**
	 * @brief Serial specific configuration
	 */
	struct {
		/**
		 * @brief Modbus slave address for this device
		 *
		 * The unique slave address identifier for this Modbus device on the serial bus.
		 * Each device on a Modbus RTU network must have a unique slave address to
		 * prevent communication conflicts. The master uses this address to direct
		 * requests to specific slave devices.
		 *
		 * @note Valid range: 1-247 (addresses 0 and 248-255 are reserved)
		 * @note Address 0 is reserved for broadcast messages
		 * @note Must be unique across all devices on the same serial bus
		 * @note Used by mbadu_handle_req() for address filtering in RTU mode
		 */
		uint8_t slave_addr;

		/**
		 * @brief Callback function for reading device exception status
		 *
		 * Optional callback function that provides device-specific exception status
		 * when Modbus function code 0x07 (Read Exception Status) is requested.
		 * This serial-only function allows the slave to report error conditions,
		 * communication issues, or diagnostic information to the master.
		 *
		 * @return 8-bit exception status value with device-specific meaning
		 *
		 * @note Can be left as NULL if exception status functionality is not needed
		 * @note Function code 0x07 is only implemented when this callback is present
		 * @note The meaning of individual status bits is device-specific
		 * @note This is a serial-only function (RTU/ASCII), not used in TCP/IP
		 * @note Common uses: hardware faults, communication errors, device warnings
		 * @note Should return consistent values for the same device state
		 */
		uint8_t (*read_exception_status_cb)(void);

		/**
		 * @brief Callback function for reading device diagnostic register
		 *
		 * Optional callback function that provides device-specific diagnostic data
		 * when Modbus diagnostic function code 0x08 with subfunction 0x0002
		 * (Return Diagnostic Register) is requested. This allows the slave to
		 * report internal diagnostic information.
		 *
		 * @return 16-bit diagnostic register value with device-specific meaning
		 *
		 * @note Can be left as NULL if diagnostic register functionality is not needed
		 * @note Used by diagnostic function 0x08 subfunction 0x0002
		 * @note The meaning of individual bits/values is device-specific
		 * @note Common uses: internal error flags, operational counters, health metrics
		 */
		uint16_t (*read_diagnostics_cb)(void);

		/**
		 * @brief Callback function for resetting device diagnostic register
		 *
		 * Optional callback function that allows the master to reset the
		 * diagnostic register when Modbus diagnostic function code 0x08
		 * with subfunction 0x000A (Clear Counters and Diagnostic Register)
		 * is requested.
		 *
		 * @note Can be left as NULL if diagnostic reset functionality is not needed
		 * @note Used by diagnostic function 0x08 subfunction 0x000A
		 * @note Common uses: resetting fault flags
		 */
		void (*reset_diagnostics_cb)(void);

		/**
		 * @brief Callback function for requesting device restart
		 *
		 * Optional callback function that is called when the master requests
		 * a device restart using Modbus diagnostic function code 0x08 with
		 * subfunction 0x0001 (Restart Communications Option).
		 *
		 * @note The reset should be queued, if the device is not in listen only
		 *       mode, a response shall be sent before restarting.
		 * @note Can be left as NULL if restart functionality is not needed
		 * @note Used by diagnostic function 0x08 subfunction 0x0001
		 * @note Common uses: resetting hardware, reinitializing communication interfaces
		 * @note The device should perform restart operations and execute power-up confidence tests
		 */
		void (*request_restart)(void);

		/**
		 * @brief Enable response to non-standard slave address 248
		 *
		 * When enabled, the device will respond to requests sent to slave
		 * address 248 in addition to its configured slave address. This is a
		 * non-standard extension useful for device identification and discovery.
		 *
		 * @note Set to 1 to enable, 0 to disable (default)
		 * @note This is not part of the Modbus specification
		 * @note Only applicable when using mbadu_handle_req() for serial communication
		 * @note Address 248 responses can help identify devices in large networks
		 */
		int enable_def_resp;
	} serial;

	/**
	 * @brief Enable little-endian word order for multi-word input registers
	 *
	 * When enabled, multi-word input register values (U32, U64, F32, F64) are
	 * transmitted with swapped word order for backward compatibility with
	 * certain legacy systems or non-standard implementations.
	 *
	 * @note Set to 1 to enable word swapping, 0 for standard big-endian (default)
	 * @note Only affects input registers (function code 0x04), not holding registers
	 */
	int swap_words;

	/**
	 * @brief Internal state for diagnostics and status tracking
	 *
	 * Contains internal state variables used by the library for tracking
	 * device status and communication events. These fields are automatically
	 * maintained by the library and support diagnostic function codes.
	 *
	 * @note Serial only
	 *
	 * @note Shall not be accessed by client code directly
	 * @note State is automatically updated during Modbus request processing
	 */
	struct {
		int is_listen_only; /**< Whether the device is in a listen only mode or not */

		uint16_t status; /**< Device status word (Not implemented) */

		/**
		 * @brief Communication event counter
		 *
		 * Incremented for each successful message completion.
		 * Provides a simple metric for monitoring communication activity.
		 *
		 * @note Automatically incremented on successful message processing
		 */
		uint16_t comm_event_counter;

		/**
		 * @brief Communication event log ring buffer
		 *
		 * Ring buffer for storing communication events and diagnostic information.
		 * Automatically overwrites oldest events when buffer is full.
		 */
		uint8_t event_log[MB_COMM_EVENT_LOG_LEN];
		int event_log_write_pos; /**< Write position in ring buffer (0 to MB_COMM_EVENT_LOG_LEN-1) */
		int event_log_count; /**< Number of events currently in buffer (0 to MB_COMM_EVENT_LOG_LEN) */

		uint16_t bus_msg_counter; /**< Total messages on bus (all devices, including CRC errors) */
		uint16_t bus_comm_err_counter; /**< CRC/LRC error count */
		uint16_t exception_counter; /**< Exception responses sent by this device */
		uint16_t msg_counter; /**< Messages addressed to this device */
		uint16_t no_resp_counter; /**< Messages to this device with no response sent */
		uint16_t nak_counter; /**< NAK exception responses sent */
		uint16_t busy_counter; /**< BUSY exception responses sent */
		uint16_t bus_char_overrun_counter; /**< Character overrun errors (not currently tracked - Could be incremented by implementation rtu) */

		/**
		 * @brief ASCII frame delimiter character for Modbus ASCII
		 *
		 * Character used to delimit the end of Modbus ASCII frames. The Modbus ASCII
		 * specification defines that frames should end with CR LF (\r\n), but this
		 * field allows customization of the line ending character.
		 *
		 * @note Default value is '\n' (0x0A - Line Feed)
		 * @note Set though function 0x08 with sub function 0x0003
		 *
		 * @note Modbus ASCII only
		 */
		uint8_t ascii_delimiter;
	} state;
};

/**
 * @brief Initialize internal instance state with default values
 *
 * Initializes all states fields of a Modbus instance structure to safe default values.
 *
 * @param inst Pointer to the Modbus instance structure to initialize
 */
extern void mbinst_init(struct mbinst_s *inst);

/**
 * @brief Add a communication event to the log
 *
 * @note Library internal function
 */
extern void mb_add_comm_event(struct mbinst_s *inst, uint8_t event);

#endif /* MBINST_H_INCLUDED */
