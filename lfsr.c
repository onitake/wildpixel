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

uint8_t lfsr_shift() {
	// operate in galois counting mode, it's more efficient
	uint8_t lsb = (uint8_t) lfsr & 1;   // get lsb (i.e., the output bit)
	lfsr >>= 1;                         // shift register
	if (lsb) {                          // if the output bit is 1, apply toggle mask (i.e. polynomial)
		// p = x³¹ + x²¹ + x¹ + x⁰
		lfsr ^= 0x80200003;
	}
	return lsb;
}

void lfsr_init(uint32_t seed) {
	// avoid lockup state
	if (seed == 0) {
		lfsr = 1;
	} else {
		lfsr = seed;
	}
	// pre-clock one full cycle
	for (uint8_t i = 0; i < 32; i++) {
		lfsr_shift();
	}
}

uint8_t lfsr_get_bit() {
	// a single bit is equal to the output of the lfsr
	return lfsr_shift();
}

uint8_t lfsr_get_byte() {
	// clock in 8 bits
	uint8_t ret = 0;
	for (uint8_t i = 0; i < 8; i++) {
		uint8_t bit = lfsr_shift();
		ret |= bit << (7 - i);
	}
	return ret;
}

uint16_t lfsr_get_short() {
	// clock in 16 bits
	uint16_t ret = 0;
	for (uint8_t i = 0; i < 16; i++) {
		uint16_t bit = lfsr_shift();
		ret |= bit << (15 - i);
	}
	return ret;
}
