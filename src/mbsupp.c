/**
 * @file mbsupp.c
 * @brief Implementation of Optional Modbus support functions
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

#include "mbsupp.h"
#include <stdint.h>

extern uint32_t mbsupp_break_us(uint32_t baud)
{
	enum {
		MICRO = 1000000u, /* Seconds to microseconds */
		BITS_IN_CHAR = 11u, /* 11 bits per character (1 start + 8 data + 1 parity + 1 stop) */
		BREAK_CHARS = 35u, /* Minimum 3.5 character times for frame detection */
		BREAK_CHARS_SCALE = 10u,
		BREAK_TIME_NUMERATOR = MICRO * BITS_IN_CHAR * BREAK_CHARS / BREAK_CHARS_SCALE,
	};

	/* It is recommended to keep break time above 1.75ms regardless of baud rate */
	enum {MIN_BREAK_TIME_US = 1750u};

	uint32_t break_us = BREAK_TIME_NUMERATOR / baud;
	return (break_us > MIN_BREAK_TIME_US) ? break_us : MIN_BREAK_TIME_US;
}
