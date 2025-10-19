/**
 * @file mbsupp.h
 * @brief Optional Modbus Support Functions
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

#ifndef MBSUPP_H_INCLUDED
#define MBSUPP_H_INCLUDED

#include <stdint.h>

/**
 * @brief Calculate Modbus RTU frame break time in microseconds
 *
 * Calculates the minimum inter-frame break time required for Modbus RTU
 * communication according to the Modbus specification. The break time is
 * based on 3.5 character times, where each character consists of 11 bits
 * (1 start + 8 data + 1 parity + 1 stop).
 *
 * @param baud Serial baud rate in bits per second (e.g., 9600, 19200, 115200)
 * @return Break time in microseconds, minimum 1750µs for higher baud rates
 *
 * @note For baud rates above ~19200, a minimum of 1.75ms is enforced as recommended
 * @note Used for RTU frame detection and inter-frame spacing in serial communication
 * @note Formula: max(1750µs, (1,000,000 * 11 * 3.5) / baud_rate)
 */
extern uint32_t mbsupp_break_us(uint32_t baud);

#endif /* MBSUPP_H_INCLUDED */
