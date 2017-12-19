/*
 * Simple LFSR based PRNG
 * Copyright 2017 Gregor Riepl <onitake@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation and/or 
 * other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lfsr.h"

static uint32_t lfsr __attribute__((section(".noinit")));

void lfsr_shift() {
	// 31, 21, 1, 0
	uint8_t x = 1;
	x ^= (lfsr >> 1) & 1;
	x ^= (lfsr >> 21) & 1;
	x ^= (lfsr >> 31) & 1;
	lfsr = (lfsr << 1) | x;
}

void lfsr_init(uint32_t seed) {
	lfsr = seed;
	uint8_t i;
	for (i = 0; i < 32; i++) {
		lfsr_shift();
	}
}

uint8_t lfsr_get_bit() {
	uint8_t ret = lfsr >> 31;
	lfsr_shift();
	return ret;
}

uint8_t lfsr_get_byte() {
	uint8_t ret = lfsr >> 24;
	uint8_t i;
	for (i = 0; i < 8; i++) {
		lfsr_shift();
	}
	return ret;
}

uint16_t lfsr_get_short() {
	uint16_t ret = lfsr >> 16;
	uint8_t i;
	for (i = 0; i < 16; i++) {
		lfsr_shift();
	}
	return ret;
}
