/**
 * @file mbascii.cpp
 * @brief Modbus ASCII Communication Implementation
 *
 * This file implements the ASCII (serial) communication layer for
 * the Arduino Modbus slave.
 */

#include "mbascii.hpp"
#include "modbus.hpp"
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

extern "C" {
#include <mbadu_ascii.h>
}

enum {TIMEOUT_MS=1000};

extern void mbascii_init(unsigned long baud)
{
	/* Initialize serial port with Modbus ASCII standard settings:
	   1 start bit, 7 data bits, even parity, 1 stop bit */
	Serial.begin(baud, SERIAL_7E1);
}

extern void mbascii_proc(void)
{
	static uint8_t s_rx[MBADU_ASCII_SIZE_MAX];
	static uint8_t s_tx[MBADU_ASCII_SIZE_MAX];
	static size_t s_rx_n = 0;

	static unsigned long s_last_recv_ms;

	struct mbinst_s *inst;
	size_t tx_n;

	if (Serial.available()) {
		if (s_rx_n >= MBADU_ASCII_SIZE_MAX) { /* Prevent buffer overflow */
			s_rx_n = 0;
		}
		s_rx[s_rx_n++] = (uint8_t)Serial.read();
		s_last_recv_ms = millis();

		inst = modbus_get();
		if (s_rx_n>MBADU_ASCII_HEADER_SIZE
				&& s_rx[s_rx_n-2]=='\r'
				&& s_rx[s_rx_n-1]==inst->state.ascii_delimiter) { /* Full request received */
			tx_n = mbadu_ascii_handle_req(inst, s_rx, s_rx_n, s_tx);

			if (tx_n) {
				Serial.write(s_tx, tx_n); /* Send response */
				Serial.flush(); /* Wait for transmission to complete */
			}

			s_rx_n = 0; /* Reset for next frame */
		}
	} else if (s_rx_n && (millis()-s_last_recv_ms) > TIMEOUT_MS) { /* Timeout */
		s_rx_n = 0;
	}
}
