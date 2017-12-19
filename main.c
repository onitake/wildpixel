/*
 * wildpixel demo platform
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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include "config.h"
#include "ws2812.h"
#include "lfsr.h"
#include "tinymath.h"

// Blends two colors together.
// Equivalent to: a * (1.0 - v) + b * v
// v ranges from 0 to 255 and is mapped to 0.0..1.0.
// No floating point arithmetic is used and the result is mapped back to 0..255.
static inline __attribute__((always_inline)) rgb_t rgb_mix(rgb_t a, rgb_t b, uint8_t v) {
	uint8_t dv = 255 - v;
	rgb_t d;
	d.r = (uint8_t) ((a.r * (uint16_t) dv + b.r * (uint16_t) v) / 256);
	d.g = (uint8_t) ((a.g * (uint16_t) dv + b.g * (uint16_t) v) / 256);
	d.b = (uint8_t) ((a.b * (uint16_t) dv + b.b * (uint16_t) v) / 256);
	return d;
}

rgb_t map_color(uint8_t index) {
	// not quite what the name says rgb-wise, but these LEDs have a terrible
	// color spectrum.
	static const rgb_t black = { 0, 0, 0 };
	static const rgb_t red = { 255, 0, 0 };
	static const rgb_t orange = { 255, 63, 0 };
	static const rgb_t yellow = { 255, 127, 0 };
	static const rgb_t white = { 255, 127, 63 };
	// scale a bit (x4?)
	acc_sat_u8(index, index);
	acc_sat_u8(index, index);
	//acc_sat_u8(index, add_sat_u8(index, index));
	// construct a piecewise linear palette
	if (index < 128) {
		// red
		return rgb_mix(black, red, (index - 0) * (256 / (128 - 0)));
	} else if (index >= 128 && index < 192) {
		// orange
		return rgb_mix(red, orange, (index - 128) * (256 / (192 - 128)));
	} else if (index >= 192 && index < 224) {
		// yellow
		return rgb_mix(orange, yellow, (index - 192) * (256 / (224 - 192)));
	} else {
		// white
		return rgb_mix(yellow, white, (index - 224) * (256 / (256 - 224)));
	}
}

int main() {
	pal_init();
	ws_init();
	
	lfsr_init(0);
	//srand(0);
	
	while (true) {
		// spawn a new spark at the bottom row
		uint8_t p = lfsr_get_byte();
		//uint8_t p = rand();
		p %= WS_WIDTH;
		ws_set_pixel(p, WS_HEIGHT - 1, add_sat_u8(ws_get_pixel(p, WS_HEIGHT - 1), 8));
		
		for (uint8_t y = WS_HEIGHT; y-- > 0;) {
			for (uint8_t x = 0; x < WS_WIDTH; x++) {
				// weighted blur
				uint8_t c = ws_get_pixel(x, y);
				uint8_t xm = x > 0 ? ws_get_pixel(x - 1, y) : 0;
				uint8_t xp = x < WS_WIDTH - 1 ? ws_get_pixel(x + 1, y) : 0;
				uint8_t ym = y > 0 ? ws_get_pixel(x, y - 1) : 0;
				uint8_t yp = y < WS_HEIGHT - 1 ? ws_get_pixel(x, y + 1) : c;
				uint16_t a = c * 127;
				a += xm * 31;
				a += xp * 31;
				a += ym * 7;
				a += yp * 63;
				
				// look up the new color and assign it
				ws_set_pixel(x, y, (uint8_t) (a / 256));
			}
		}
		
		ws_scan_fb();
		//_delay_ms(150);
	}
}
