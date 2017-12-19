/*
 * Framebuffer module for WS-2812 chip LEDs
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

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include "ws2812.h"

// C preproc pasting magic
#define _WS_CONCAT2(x, y) x##y
#define _WS_CONCAT(x, y) _WS_CONCAT2(x, y)
// Some internal conveniences
#define _WS_REG_PORT _WS_CONCAT(PORT, WS_PORT)
#define _WS_REG_DDR _WS_CONCAT(DDR, WS_PORT)
// the total size of the frame buffer
#define _WS_FB_SIZE ((WS_WIDTH) * (WS_HEIGHT))
// number of chains: width * height / chain length
#define _WS_NUM_CHAINS ((_WS_FB_SIZE) / (WS_CHAIN))

uint8_t ws_fb[_WS_FB_SIZE] __attribute__((section(".noinit")));

// Latch timing, the data sheet says >50us
// Oh, and a latch is not a reset, by the way.
#define WS_T_RES 60.0
// The timings in the data sheet are wrong and inconsistent (t1 != t0).
// What the fuck were these freaks smoking?
// The correct timings are courtesy of Adafruit. Thanks!
// With these values, they sum up to 1.25us and fit nicely into
// 20 clock cycles at 16MHz.
#define WS_T_0H 0.4375
#define WS_T_0L 0.8125
#define WS_T_1H 0.8125
#define WS_T_1L 0.4375

#define _ws_latch(port, bit) \
	PORT##port &= ~_BV(bit); \
	_delay_us(WS_T_RES);
static inline void _ws_write_real(uint8_t port, uint8_t high, uint8_t pattern, uint8_t low) {
	asm volatile (
		// write stage a: all high T=0
		"out %[Port], %[High]\n\t"
		// wait 6 clock cycles (+ 1 cycle for out = 7) T=1
		"rjmp .+0\n\t"
		"rjmp .+0\n\t"
		"rjmp .+0\n\t"
		// write stage b: bit pattern T=7
		"out %[Port], %[Pattern]\n\t"
		// wait 5 more clock cycles (+ out = 6) T=8
		"rjmp .+0\n\t"
		"rjmp .+0\n\t"
		"nop\n\t"
		// write stage c: all low T=13
		"out %[Port], %[Low]\n\t"
		// wait 7 clock cycles T=14
		"rjmp .+0\n\t"
		"rjmp .+0\n\t"
		"rjmp .+0\n\t"
		"nop\n\t"
		// transfer complete T=21
		:
		: [High] "r" (high), [Pattern] "r" (pattern), [Low] "r" (low), [Port] "I" (port)
		:
	);
}
#define _ws_write(port, high, pattern, low) _ws_write_real(_SFR_IO_ADDR(port), high, pattern, low)

void ws_init() {
	_WS_REG_PORT &= ~0xfc;
	_WS_REG_DDR |= 0xfc;
	memset(ws_fb, 0, WS_WIDTH * WS_HEIGHT * sizeof(ws_fb[0]));
}

void ws_scan_fb() {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// fetch current port state and mask unused outputs
		uint8_t low = _WS_REG_PORT & ~0xfc;
		// apply all-on pattern
		uint8_t high = low | 0xfc;
		// TODO test pattern
#if 0
		for (uint8_t i = 0; i < 8; i++) {
			uint8_t pattern = low | 0x04;
			_ws_write(D, high, pattern, low);
		}
#endif
		for (uint8_t p = 0; p < WS_CHAIN; p++) {
			// fetch a color from the frame buffer
			rgb_t rgb = pal_lookup(ws_fb[p]);
			// arrayize and flip
			uint8_t grb[] = { rgb.g, rgb.r, rgb.b };
			//uint8_t grb[] = { 255, 0, 0 };
			for (uint8_t c = 0; c < 3; c++) {
				uint8_t g = grb[c];
				for (uint8_t b = 0; b < 8; b++) {
					// apply pixel pattern (during transition period high-low)
					uint8_t pattern = low;
					pattern |= (g & 0x80) ? 0x04 : 0x00;
					_ws_write(_WS_REG_PORT, high, pattern, low);
					g <<= 1;
				}
			}
		}
#if 0
		// loop over the first chain - the other chains are referenced relative to that
		for (uint8_t p = 0; p < fb->chain; p++) {
			ws_rgb_t *pixel0 = &fb->buffer[p];
			for (uint8_t c = 0; c < 3; c++) {
				uint8_t *component = &pixel0->grb[c];
				for (uint8_t b = 0; b < 8; b++) {
					// apply pixel pattern (during transition period high-low)
					uint8_t pattern = low;
					for (uint8_t n = 0; n < nchains; n++) {
						pattern |= ((component[n * stride] >> (7 - b)) & 1) << (2 + n);
					}
					_ws_write(D, high, pattern, low);
				}
			}
		}
#endif
	}
	// latch
	_delay_us(WS_T_RES);
}
