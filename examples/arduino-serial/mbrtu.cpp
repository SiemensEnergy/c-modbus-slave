/**
 * @file mbrtu.cpp
 * @brief Modbus RTU Communication Implementation
 *
 * This file implements the RTU (serial) communication layer for
 * the Arduino Modbus slave.
 */

#include "mbrtu.hpp"
#include "modbus.hpp"
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

extern "C" {
#include <mbadu.h>
#include <mbsupp.h>
}

static size_t s_break_us = 0; /* Frame break time in microseconds */

extern void mbrtu_init(unsigned long baud)
{
	/* Initialize serial port with Modbus RTU standard settings:
	   8 data bits, even parity, 1 stop bit */
	Serial.begin(baud, SERIAL_8E1);

	s_break_us = mbsupp_break_us(baud);
}

extern void mbrtu_proc(void)
{
	static uint8_t s_rx[MBADU_SIZE_MAX];
	static uint8_t s_tx[MBADU_SIZE_MAX];
	static size_t s_rx_n = 0;

	static unsigned long s_last_recv_us;

	size_t tx_n;

	if (Serial.available()) {
		if (s_rx_n >= MBADU_SIZE_MAX) { /* Prevent buffer overflow */
			s_rx_n = 0;
		}
		s_rx[s_rx_n++] = (uint8_t)Serial.read();
		s_last_recv_us = micros();
	} else if (s_rx_n && (micros()-s_last_recv_us) >= s_break_us) { /* Check for frame completion (no data for break time) */
		tx_n = mbadu_handle_req(modbus_get(), s_rx, s_rx_n, s_tx);

		if (tx_n) {
			Serial.write(s_tx, tx_n); /* Send response */
			Serial.flush(); /* Wait for transmission to complete */
		}

		s_rx_n = 0; /* Reset for next frame */
	}
}
