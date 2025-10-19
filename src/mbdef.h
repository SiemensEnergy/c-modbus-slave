/**
 * @file mbdef.h
 * @brief Modbus definitions
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

#ifndef MBDEF_H_INCLUDED
#define MBDEF_H_INCLUDED

/** Modbus status code */
enum mbstatus_e {
	MB_OK = 0x00u,
	MB_ILLEGAL_FN = 0x01u, /* Function code received in the query is not recognized or allowed. */
	MB_ILLEGAL_DATA_ADDR = 0x02u, /* Data address of some or all the required entities are not allowed or do not exist.*/
	MB_ILLEGAL_DATA_VAL = 0x03u, /* Illegal data value. Value is not accepted. */
	MB_DEV_FAIL = 0x04u, /* Unrecoverable error occurred while slave was attempting to perform requested action. */
	/* Slave has accepted request and is processing it, but a long duration of time is required.
	   This response is returned to prevent a timeout error from occurring on the master.
	   The master can next issue a Poll Program Complete message to determine whether processing is completed */
	MB_ACK = 0x05u,
	MB_BUSY = 0x06u, /* Slave is engaged in processing a long-duration command; client should retry later */
	/* Slave cannot perform the programming functions;
	   master should request diagnostic or error information from slave */
	MB_NEG_ACK = 0x07u,
	MB_MEM_PAR_ERR = 0x08u, /* Slave detected a parity error in memory; master can retry the request */
};

/** Modbus function code */
enum mbfc_e {
	MBFC_READ_COILS = 0x01u,
	MBFC_READ_DISC_INPUTS = 0x02u,
	MBFC_READ_HOLDING_REGS = 0x03u,
	MBFC_READ_INPUT_REGS = 0x04u,
	MBFC_WRITE_SINGLE_COIL = 0x05u,
	MBFC_WRITE_SINGLE_REG = 0x06u,
	MBFC_READ_EXCEPTION_STATUS = 0x07u, /* Serial only */
	MBFC_DIAGNOSTICS = 0x08u, /* Serial only */
	MBFC_COMM_EVENT_COUNTER = 0x0Bu, /* Serial only */
	MBFC_COMM_EVENT_LOG = 0x0Cu, /* Serial only */
	MBFC_WRITE_MULTIPLE_COILS = 0x0Fu,
	MBFC_WRITE_MULTIPLE_REGS = 0x10u,
	MBFC_REPORT_SLAVE_ID = 0x11u, /* Serial only */
	MBFC_READ_FILE_RECORD = 0x14u,
	MBFC_WRITE_FILE_RECORD = 0x15u,
	MBFC_MASK_WRITE_REG = 0x16u,
	MBFC_READ_WRITE_REGS = 0x17u,
	MBFC_READ_FIFO_QUEUE = 0x18u,
};

/** Modbus diagnostics sub function code */
enum mbfn_digs_e {
	MBFC_DIGS_LOOPBACK = 0x00u, /* Return Query Data */
	MBFC_DIGS_RESTART_COMMS_OPT = 0x01u, /* Restart Communications Option */
	MBFC_DIGS_REG = 0x02u, /* Return Diagnostic Register */
	MBFC_DIGS_ASCII_DELIM = 0x03u, /* Change ASCII Input Delimiter */
	MBFC_DIGS_FORCE_LISTEN = 0x04u, /* Force Listen Only Mode */
	/* 0x05..0x09 - Reserved */
	MBFC_DIGS_CLR_CNTS_N_DIAG_REG = 0x0Au, /* Clear Counters and Diagnostic Register */
	MBFC_DIGS_BUS_MSG_COUNT = 0x0Bu, /* Return Bus Message Count */
	MBFC_DIGS_BUS_COMM_ERR_COUNT = 0x0Cu, /* Return Bus Communication Error Count */
	MBFC_DIGS_BUS_EXCEPTION_COUNT = 0x0Du, /* Return Bus Exception Error Count */
	MBFC_DIGS_MSG_COUNT = 0x0Eu, /* Return Server Message Count */
	MBFC_DIGS_NO_RESP_MSG_COUNT = 0x0Fu, /* Return Server No Response Count */
	MBFC_DIGS_NAK_COUNT = 0x10u, /* Return Server NAK Count */
	MBFC_DIGS_BUSY_COUNT = 0x11u, /* Return Server Busy Count */
	MBFC_DIGS_BUS_OVERRUN_COUNT = 0x12u, /* Return Bus Character Overrun Count */
	/* 0x13 - Reserved */
	MBFC_DIGS_CLR_OVERRUN = 0x14u, /* Clear Overrun Counter and Flag */
	/* 0x15-0xFFFF - Reserved */
};

enum {
	/*
	 * Modbus error flag
	 * Added onto the function code in an error response
	 */
	MB_ERR_FLG=0x80u
};

enum {MB_COMM_EVENT_LOG_LEN=64};

enum { /* Communication log event */
	/* Receive event*/
	MB_COMM_EVENT_IS_RECV = 1u<<7, /* Indicated whether the event is a recv event or not */

	/* 0 - Not used */
	MB_COMM_EVENT_RECV_COMM_ERR = 1u<<1, /* Communication Error */
	/* 2 - Not used */
	/* 3 - Not used */
	MB_COMM_EVENT_RECV_CHAR_OVERRUN = 1u<<4, /* Character Overrun */
	MB_COMM_EVENT_RECV_LISTEN_MODE = 1u<<5, /* Currently in listen mode */
	MB_COMM_EVENT_RECV_BROADCAST = 1u<<6, /* Broadcast Received */

	/* Send event */
	MB_COMM_EVENT_SEND_READ_EX = 1u<<0, /* Read Exception Sent (Exception Codes 1-3) */
	MB_COMM_EVENT_SEND_ABORT_EX = 1u<<1, /* Server Abort Exception Sent (Exception Code 4) */
	MB_COMM_EVENT_SEND_BUSY_EX = 1u<<2, /* Server Busy Exception Sent (Exception Codes 5-6) */
	MB_COMM_EVENT_SEND_NAK_EX = 1u<<3, /* Server Program NAK Exception Sent (Exception Code 7) */
	MB_COMM_EVENT_SEND_WRITE_TIMEOUT = 1u<<4, /* Write Timeout Error Occurred (Not implemented) */
	MB_COMM_EVENT_SEND_LISTEN_ONLY = 1u<<5, /* Currently in Listen Only Mode */
	MB_COMM_EVENT_IS_SEND = 1u<<6, /* Indicated whether the event is a send event or not, requires MB_COMM_IS_RECV_EVENT to be 0 */

	/* Other events */
	MB_COMM_EVENT_COMM_RESTART=0x00u,
	MB_COMM_EVENT_ENTERED_LISTEN_ONLY=0x04u,
};

#endif /* MBDEF_H_INCLUDED */
