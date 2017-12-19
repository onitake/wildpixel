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

#ifndef _WS2812_H
#define _WS2812_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "palette.h"

// Supported configuration variables:
// WS_WIDTH: Total width of the display in pixels
#ifndef WS_WIDTH
#error WS_WIDTH must be defined
#endif
// WS_WIDTH: Total height of the display in pixels
#ifndef WS_HEIGHT
#error WS_HEIGHT must be defined
#endif
// WS_CHAIN: Number of pixels per LED chain.
// Defaults to WS_WIDTH x WS_HEIGHT
#ifndef WS_CHAIN
#define WS_CHAIN (WS_WIDTH * WS_HEIGHT)
#endif
// WS_ZIGZAG: Define if LED rows are connected head-to-head and tail-to-tail.
// Defaults to undefined; rows are all scanned in the same direction.
//#undef WS_ZIGZAG
// WS_PORT: The GPIO port to use.
// This should be a single letter, not the PORTx register.
// The following pins are available on the Arduino Uno/Pro:
// PD2..7 (IO2..7)
// PC0..5 (AD0..5)
// PB0..1 (IO8..9)
#ifndef WS_PORT
#error WS_PORT must be defined
#endif
// WS_FIRST_PIN: The first GPIO pin number to use.
// Defaults to 0.
#ifndef WS_FIRST_PIN
#define WS_FIRST_PIN 0
#endif

// The frame buffer
extern uint8_t ws_fb[];

// Returns the index of a specific pixel inside the framebuffer.
#define ws_get_pixel_index(x, y, width) ((y) * (width) + (x))

// Initializes the IO ports and clears the frame buffer
void ws_init();

// Scans the frame buffer once and latches the outputs.
// Should be called from a timer.
void ws_scan_fb();

// Sets the pixel at x,y to a color value index
static inline void ws_set_pixel(uint8_t x, uint8_t y, uint8_t index) {
	ws_fb[ws_get_pixel_index(x, y, WS_WIDTH)] = index;
}

// Gets the color value of the pixel at x,y
static inline uint8_t ws_get_pixel(uint8_t x, uint8_t y) {
	return ws_fb[ws_get_pixel_index(x, y, WS_WIDTH)];
}

#endif /*_WS2812_H*/
