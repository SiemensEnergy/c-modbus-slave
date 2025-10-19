/**
 * @file endian.c
 * @brief Implementation of endian conversion utilities
 * @author Jonas Almås
 *
 * MISRA Deviations:
 * - Rule 11.3: A conversion shall not be performed between a pointer to object type and a pointer to a different object type
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

#include "endian.h"

extern uint16_t betou16(const uint8_t *buf)
{
	return ((uint16_t)buf[0] << 8)
		| ((uint16_t)buf[1]);
}

extern uint32_t betou32(const uint8_t *buf)
{
	return ((uint32_t)buf[0] << 24)
		| ((uint32_t)buf[1] << 16)
		| ((uint32_t)buf[2] << 8)
		| ((uint32_t)buf[3]);
}

extern uint64_t betou64(const uint8_t *buf)
{
	return ((uint64_t)buf[0] << 56)
		| ((uint64_t)buf[1] << 48)
		| ((uint64_t)buf[2] << 40)
		| ((uint64_t)buf[3] << 32)
		| ((uint64_t)buf[4] << 24)
		| ((uint64_t)buf[5] << 16)
		| ((uint64_t)buf[6] << 8)
		| ((uint64_t)buf[7]);
}

extern int16_t betoi16(const uint8_t *buf)
{
	uint16_t tmp = betou16(buf);
	return *(int16_t *)&tmp;
}

extern int32_t betoi32(const uint8_t *buf)
{
	uint32_t tmp = betou32(buf);
	return *(int32_t *)&tmp;
}

extern int64_t betoi64(const uint8_t *buf)
{
	uint64_t tmp = betou64(buf);
	return *(int64_t *)&tmp;
}

extern float betof32(const uint8_t *buf)
{
	uint32_t tmp = betou32(buf);
	return *(float *)&tmp;
}

extern double betof64(const uint8_t *buf)
{
	uint64_t tmp = betou64(buf);
	return *(double *)&tmp;
}

extern uint16_t letou16(const uint8_t *buf)
{
	return (uint16_t)buf[0] | (uint16_t)buf[1] << 8;
}

extern uint32_t letou32(const uint8_t *buf)
{
	return (uint32_t)buf[0]
		| (uint32_t)buf[1] << 8
		| (uint32_t)buf[2] << 16
		| (uint32_t)buf[3] << 24;
}

extern uint64_t letou64(const uint8_t *buf)
{
	return (uint64_t)buf[0]
		| (uint64_t)buf[1] << 8
		| (uint64_t)buf[2] << 16
		| (uint64_t)buf[3] << 24
		| (uint64_t)buf[4] << 32
		| (uint64_t)buf[5] << 40
		| (uint64_t)buf[6] << 48
		| (uint64_t)buf[7] << 56;
}

extern int16_t letoi16(const uint8_t *buf)
{
	uint16_t tmp = letou16(buf);
	return *(int16_t *)&tmp;
}

extern int32_t letoi32(const uint8_t *buf)
{
	uint32_t tmp = letou32(buf);
	return *(int32_t *)&tmp;
}

extern int64_t letoi64(const uint8_t *buf)
{
	uint64_t tmp = letou64(buf);
	return *(int64_t *)&tmp;
}

extern float letof32(const uint8_t *buf)
{
	uint32_t tmp = letou32(buf);
	return *(float *)&tmp;
}

extern double letof64(const uint8_t *buf)
{
	uint64_t tmp = letou64(buf);
	return *(double *)&tmp;
}

extern void u16tobe(uint16_t val, uint8_t *dst)
{
	dst[0] = (uint8_t)((val >> 8) & 0xFFu);
	dst[1] = (uint8_t)(val & 0xFFu);
}

extern void u32tobe(uint32_t val, uint8_t *dst)
{
	dst[0] = (uint8_t)((val >> 24) & 0xFFu);
	dst[1] = (uint8_t)((val >> 16) & 0xFFu);
	dst[2] = (uint8_t)((val >> 8) & 0xFFu);
	dst[3] = (uint8_t)(val & 0xFFu);
}

extern void u64tobe(uint64_t val, uint8_t *dst)
{
	dst[0] = (uint8_t)((val >> 56) & 0xFFu);
	dst[1] = (uint8_t)((val >> 48) & 0xFFu);
	dst[2] = (uint8_t)((val >> 40) & 0xFFu);
	dst[3] = (uint8_t)((val >> 32) & 0xFFu);
	dst[4] = (uint8_t)((val >> 24) & 0xFFu);
	dst[5] = (uint8_t)((val >> 16) & 0xFFu);
	dst[6] = (uint8_t)((val >> 8) & 0xFFu);
	dst[7] = (uint8_t)(val & 0xFFu);
}

extern void i16tobe(int16_t val, uint8_t *dst)
{
	u16tobe(*(uint16_t *)&val, dst);
}

extern void i32tobe(int32_t val, uint8_t *dst)
{
	u32tobe(*(uint32_t *)&val, dst);
}

extern void i64tobe(int64_t val, uint8_t *dst)
{
	u64tobe(*(uint64_t *)&val, dst);
}

extern void f32tobe(float val, uint8_t *dst)
{
	u32tobe(*(uint32_t *)&val, dst);
}

extern void f64tobe(double val, uint8_t *dst)
{
	u64tobe(*(uint64_t *)&val, dst);
}

extern void u16tole(uint16_t val, uint8_t *dst)
{
	dst[0] = (uint8_t)(val & 0xFFu);
	dst[1] = (uint8_t)((val >> 8) & 0xFFu);
}

extern void u32tole(uint32_t val, uint8_t *dst)
{
	dst[0] = (uint8_t)(val & 0xFFu);
	dst[1] = (uint8_t)((val >> 8) & 0xFFu);
	dst[2] = (uint8_t)((val >> 16) & 0xFFu);
	dst[3] = (uint8_t)((val >> 24) & 0xFFu);
}

extern void u64tole(uint64_t val, uint8_t *dst)
{
	dst[0] = (uint8_t)(val & 0xFFu);
	dst[1] = (uint8_t)((val >> 8) & 0xFFu);
	dst[2] = (uint8_t)((val >> 16) & 0xFFu);
	dst[3] = (uint8_t)((val >> 24) & 0xFFu);
	dst[4] = (uint8_t)((val >> 32) & 0xFFu);
	dst[5] = (uint8_t)((val >> 40) & 0xFFu);
	dst[6] = (uint8_t)((val >> 48) & 0xFFu);
	dst[7] = (uint8_t)((val >> 56) & 0xFFu);
}

extern void i16tole(int16_t val, uint8_t *dst)
{
	u16tole(*(uint16_t *)&val, dst);
}

extern void i32tole(int32_t val, uint8_t *dst)
{
	u32tole(*(uint32_t *)&val, dst);
}

extern void i64tole(int64_t val, uint8_t *dst)
{
	u64tole(*(uint64_t *)&val, dst);
}

extern void f32tole(float val, uint8_t *dst)
{
	u32tole(*(uint32_t *)&val, dst);
}

extern void f64tole(double val, uint8_t *dst)
{
	u64tole(*(uint64_t *)&val, dst);
}
