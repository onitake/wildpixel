/*
 * Signal processing functions, optimised for AVR microcontrollers
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

#ifndef _DSP_H
#define _DSP_H

#include <stdint.h>
#include <math.h>
#include "tinymath.h"

// calculate the sampling period from the sampling frequency
// dt = 1/fs
// floating point result, should only be used in constant expressions
#define dsp_sample_period(fs) (1.0 / (fs))

// processing coefficient calculation
// these macros should be called with constant expressions to avoid bringing in
// floating point math functions (which are slow).
// fc = corner frequency [Hz]
// dt = sampling period (1/fs) [s]
#define dsp_iir_lowpass_alpha(fc, dt) (uint8_t) (255.0 * ((2.0 * M_PI * (dt) * (fc)) / (2.0 * M_PI * (dt) * (fc) + 1)))
#define dsp_iir_highpass_alpha(fc, dt) (uint8_t) (255.0 * (1.0 / (2.0 * M_PI * (dt) * (fc) + 1)))

// fixed-point volume scaling
static inline int8_t dsp_sample_scale(int8_t sample, uint8_t volume) {
	return mul_fix_su8(sample, volume);
}

// infinite impulse response low-pass filter
// y1 = last output value
// x1 = last input value
// x0 = current input value
// alpha = dsp_iir_lowpass_alpha(corner frequency)
// returns y0, the current output value
static inline int8_t dsp_iir_lowpass(int8_t y1, int8_t x1, int8_t x0, uint8_t alpha) {
	//return madd_fix_su8(last, 255 - alpha, (int16_t) current * (uint16_t) alpha);
	//return blend_fix_s8(last, current, alpha);
	//return mul_fix_su8(y1, 255 - alpha) + mul_fix_su8(x0, alpha);
	return blend_fix_su8(y1, x0, alpha);
	//double a = alpha / 255.0;
	//return (int8_t) (y1 * (1.0 - a) + x0 * a);
}

// infinite impulse response low-pass filter
// y1 = last output value
// x1 = last input value
// x0 = current input value
// alpha = dsp_iir_highpass_alpha(corner frequency)
// returns y0, the current output value
static inline int8_t dsp_iir_highpass(int8_t y1, int8_t x1, int8_t x0, uint8_t alpha) {
	return mul_fix_su8(y1 + x0 - x1, alpha);
	//double a = alpha / 255.0;
	//return (int8_t) (((double) y1 + x0 - x1) * a);
}

#endif /*_DSP_H*/
