/*
 * Palette module for wildpixel
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

#ifndef _PALETTE_H
#define _PALETTE_H

#include <stdint.h>
#include "config.h"

// Supported configuration variables:
// PAL_MODE: One of PAL_MODE_ROM, PAL_MODE_RAM, PAL_MODE_CAL
#ifndef PAL_MODE
#warning PAL_MODE not defined, defaulting to PAL_MODE_ROM
#define PAL_MODE PAL_MODE_ROM
#endif
// PAL_MODE_ROM: Load color values from a static table in ROM.
// Requires PAL_ROM_INC and PAL_TABLE_VAR.
#define PAL_MODE_ROM 1
// PAL_MODE_RAM: Load color values from a table in RAM, calculated at startup.
// Requires PAL_TABLE_VAR and PAL_COMPUTE.
#define PAL_MODE_RAM 2
// PAL_MODE_CAL: Calculate color values in-place.
// Requires PAL_COMPUTE.
#define PAL_MODE_CAL 3
// PAL_ROM_INC: Name of a header file containing a static palette.
// Must reside in ROM.
#if PAL_MODE == PAL_MODE_ROM && !defined(PAL_ROM_INC)
#warning PAL_ROM_INC not defined, defaulting to "palette_rom.h"
#define PAL_ROM_INC "palette_rom.h"
#endif
// PAL_TABLE_VAR: Variable name of the palette in ROM or RAM.
// If ROM mode is used, this variable needs to exist in PAL_ROM_INC and
// be declared to reside in ROM.
// Type: const PROGMEM pal_t
// In RAM mode, this variable will be implicitly declared in palette.c.
#if (PAL_MODE == PAL_MODE_ROM || PAL_MODE == PAL_MODE_RAM) && !defined(PAL_TABLE_VAR)
#warning PAL_TABLE_VAR not defined, defaulting to pal_table
#define PAL_TABLE_VAR pal_table
#endif
// PAL_COMPUTE: Name of a palette computation function.
// Signature: rgb_t pal_compute(uint8_t index)
#if (PAL_MODE == PAL_MODE_RAM || PAL_MODE == PAL_MODE_CAL) && !defined(PAL_COMPUTE)
#warning PAL_COMPUTE not defined, defaulting to pal_compute
#define PAL_COMPUTE pal_compute
#endif

// Data structure for RGB888 color data.
typedef struct {
	uint8_t r, g, b;
} rgb_t;

// Palette array: Maps color indices to RGB colors
// Can live either in RAM or ROM, depending on the palette lookup mode.
typedef rgb_t pal_t[256];

// Generates a color in-place
static inline rgb_t rgb_mk(uint8_t r, uint8_t g, uint8_t b) {
	rgb_t color = { .r = r, .g = g, .b = b };
	return color;
}

#if PAL_MODE == PAL_MODE_ROM
#include <avr/pgmspace.h>
extern const PROGMEM pal_t PAL_TABLE_VAR;
static inline rgb_t _pal_lookup_rom(uint8_t index) {
	rgb_t color;
	color.r = pgm_read_byte(&PAL_TABLE_VAR[index].r);
	color.g = pgm_read_byte(&PAL_TABLE_VAR[index].g);
	color.b = pgm_read_byte(&PAL_TABLE_VAR[index].b);
	return color;
}
#elif PAL_MODE == PAL_MODE_RAM
extern pal_t PAL_TABLE_VAR;
rgb_t PAL_COMPUTE(uint8_t index);
void _pal_init_ram();
static inline rgb_t _pal_lookup_ram(uint8_t index) {
	return PAL_TABLE_VAR[index];
}
#elif PAL_MODE == PAL_MODE_CAL
rgb_t PAL_COMPUTE(uint8_t index);
static inline rgb_t _pal_lookup_cal(uint8_t index) {
	return PAL_COMPUTE(index);
}
#else
#error Invalid palette lookup mode
#endif

// Initialize the palette. Only used in RAM mode.
static inline void pal_init() {
#if PAL_MODE == PAL_MODE_ROM
	// nop
#elif PAL_MODE == PAL_MODE_RAM
	_pal_init_ram();
#elif PAL_MODE == PAL_MODE_CAL
	// nop
#endif
}

// Look up a color value from ROM or RAM, or calculate in-place.
static inline rgb_t pal_lookup(uint8_t index) {
#if PAL_MODE == PAL_MODE_ROM
	return _pal_lookup_rom(index);
#elif PAL_MODE == PAL_MODE_RAM
	return _pal_lookup_ram(index);
#elif PAL_MODE == PAL_MODE_CAL
	return _pal_lookup_cal(index);
#endif
}

#endif /*_PALETTE_H*/
