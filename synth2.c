/*
 * Noise generator test program
 * Copyright 2017 Gregor Riepl <onitake@gmail.com>
 *
 * Compile with: gcc -Wall -O0 -g -o synth2 $(pkg-config --cflags --libs libpulse-simple) synth2.c lfsr.c
 * To be used on the host system, not a µC.
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pulse/simple.h>
#include "lfsr.h"

// sampling frequency (Hz)
#define SMP_FREQ 44100
// time base (1/f)
#define SMP_DT (1.0 / SMP_FREQ)
// number of sample to average (for gaussian distribution)
#define SMP_GAUSS 2
// sample volume (0..255)
#define SMP_VOLUME 60
// sample buffer size
#define SMP_BUFFER (SMP_FREQ / 2)

#define fir_mk_lowpass_alpha(f) (uint8_t) (255.0 * ((2.0 * M_PI * SMP_DT * f) / (2.0 * M_PI * SMP_DT * f + 1)))
#define fir_mk_highpass_alpha(f) (uint8_t) (255.0 * (1.0 / (2.0 * M_PI * SMP_DT * f + 1)))

// scaled fixed point 8x8=8 bit multiply
uint8_t mul_fix8(uint8_t x, uint8_t y) {
	return (uint8_t) (((uint16_t) x * y) >> 8);
}

// scaled fixed point 8x8+16=8 bit multiply and add
// note: may overflow when product and addend are both larger than 0x7fff
uint8_t madd_fix8(uint8_t x, uint8_t y, uint16_t a) {
	return (uint8_t) (((uint16_t) x * (uint16_t) y + (uint16_t) a) >> 8);
}

uint8_t blend(uint8_t a, uint8_t b, uint8_t alpha) {
	return madd_fix8(a, 255 - alpha, (uint16_t) b * alpha);
}

uint8_t rnd_get() {
	//return (uint8_t) rand();
	return lfsr_get_byte();
}

uint8_t rnd_gaussian() {
	uint16_t sum = 0;
	for (size_t i = 0; i < SMP_GAUSS; i++) {
		sum += rnd_get();
	}
	return (uint8_t) (sum / SMP_GAUSS);
}

// fixed-point volume scaling
uint8_t sample_scale(uint8_t sample, uint8_t volume) {
	return mul_fix8(sample, volume);
}

// finite impulse response low-pass filter
// alpha must be calculated with fir_mk_lowpass_alpha
uint8_t fir_lowpass(uint8_t last, uint8_t current, uint8_t alpha) {
	return blend(last, current, alpha);
}

// finite impulse response low-pass filter
// alpha must be calculated with fir_mk_highpass_alpha
// FIXME produces nasty results for low frequencies, presumably due to integer overflow
uint8_t fir_highpass(uint8_t last, uint8_t diff, uint8_t alpha) {
	return mul_fix8(last + diff, alpha);
}

int main(int argc, char **argv) {
	//srand(0);
	lfsr_init(1);
	uint8_t *buffer = calloc(SMP_BUFFER, 1);
	
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_U8;
	ss.channels = 1;
	ss.rate = SMP_FREQ;
	pa_simple *s = pa_simple_new(
		NULL,               // Use the default server.
		argv[0],            // Our application's name.
		PA_STREAM_PLAYBACK,
		NULL,               // Use the default device.
		"noise",            // Description of our stream.
		&ss,                // Our sample format.
		NULL,               // Use default channel map
		NULL,               // Use default buffering attributes.
		NULL                // Ignore error code.
	);
	
	uint16_t fc = 0;
	while (1) {
		uint8_t alpha = fir_mk_lowpass_alpha(fc);
		uint8_t alpha2 = fir_mk_highpass_alpha(fc);
		printf("fc = %u alpha = %u alpha2 = %u\n", fc, alpha, alpha2);
		uint8_t prev = 0;
		for (size_t i = 0; i < SMP_BUFFER; i++) {
			uint8_t r = rnd_gaussian();
			uint8_t v = sample_scale(r, SMP_VOLUME);
			uint8_t c = prev;
			prev = v;
			if (i >= 1) {
				v = fir_lowpass(buffer[i - 1], v, alpha);
//				v = fir_highpass(buffer[i - 1], v - c, alpha2);
			}
			uint8_t s = v;
			buffer[i] = s;
		}
		// frequency step (30Hz)
		fc = (fc + 30) % 10000;
		// send for playback
		pa_simple_write(
			s,
			buffer,
			SMP_BUFFER,
			NULL
		);
	}
	
	pa_simple_drain(s, NULL);
	pa_simple_free(s);
	free(buffer);
	
	return 0;
}
