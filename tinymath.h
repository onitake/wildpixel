/*
 * Fast & tiny integer math library for AVR µCs
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

#ifndef _TINYMATH_H
#define _TINYMATH_H

#include <stdint.h>
#include <avr/pgmspace.h>

extern const PROGMEM uint8_t tiny_sintable_64[];

static inline int8_t fastsin8(uint8_t angle) {
  uint8_t quadrant = angle >> 6;
  switch (quadrant) {
    case 0:
      return pgm_read_byte(&tiny_sintable_64[angle & 0x3f]);
    case 1:
      return pgm_read_byte(&tiny_sintable_64[63 - (angle & 0x3f)]);
    case 2:
      return -pgm_read_byte(&tiny_sintable_64[(angle & 0x3f)]);
    case 3:
      return -pgm_read_byte(&tiny_sintable_64[(63 - (angle & 0x3f))]);
  }
  return 0;
}

static inline int8_t fastcos8(uint8_t angle) {
  uint8_t quadrant = angle >> 6;
  switch (quadrant) {
    case 3:
      return pgm_read_byte(&tiny_sintable_64[angle & 0x3f]);
    case 0:
      return pgm_read_byte(&tiny_sintable_64[63 - (angle & 0x3f)]);
    case 1:
      return -pgm_read_byte(&tiny_sintable_64[(angle & 0x3f)]);
    case 2:
      return -pgm_read_byte(&tiny_sintable_64[(63 - (angle & 0x3f))]);
  }
  return 0;
}

// Saturated add on uint8_t
// a + b > 255? -> 255
// else         -> a + b
#define add_sat_u8(a, b) ((((uint16_t) a) + (b) > 255) ? 255 : ((a) + (b)))
// Saturated accumulate on uint8_t
// a + b > 255? -> a = 255
// else         -> a = a + b
#define acc_sat_u8(a, b) (a) = (((uint16_t) (a) + (b) > 255) ? 255 : ((a) + (b)))

// some handy 8bit fixed-point operations
// these require a hardware 8x8=16 multiplier for efficient operation

// scaled fixed point 8x8=8 bit multiply
static inline uint8_t mul_fix8(uint8_t x, uint8_t y) {
	return (uint8_t) (((uint16_t) x * y) >> 8);
}

// scaled fixed point 8x8+16=8 bit multiply and add
// note: may overflow when product and addend are both larger than 0x7fff
static inline uint8_t madd_fix8(uint8_t x, uint8_t y, uint16_t a) {
	return (uint8_t) (((uint16_t) x * (uint16_t) y + (uint16_t) a) >> 8);
}

// scaled fixed point 8x8+8x8=8 blend function
static inline uint8_t blend_fix8(uint8_t a, uint8_t b, uint8_t alpha) {
	return madd_fix8(a, 255 - alpha, (uint16_t) b * alpha);
}

#endif /*_TINYMATH_H*/
