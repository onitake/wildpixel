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

#endif /*_TINYMATH_H*/
