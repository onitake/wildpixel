/*
 * Noise generator test program
 * Copyright 2017 Gregor Riepl <onitake@gmail.com>
 *
 * Compile with: gcc -Wall -O0 -g -o synth2 $(pkg-config --cflags --libs libpulse-simple) synth2.c lfsr.c tinymath.c
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
#include <fcntl.h>
#include <unistd.h>
#include <pulse/simple.h>
#include "lfsr.h"
#include "tinymath.h"

// sampling frequency (Hz)
#define SMP_FREQ 44100
// time base (1/f)
#define SMP_DT (1.0 / SMP_FREQ)
// number of sample to average (for gaussian distribution)
#define SMP_GAUSS 4
// sample volume (0..255)
#define SMP_VOLUME 255
// sample buffer size
#define SMP_BUFFER (SMP_FREQ / 2)

#define fir_mk_lowpass_alpha(f) (uint8_t) (255.0 * ((2.0 * M_PI * SMP_DT * (f)) / (2.0 * M_PI * SMP_DT * (f) + 1)))
#define fir_mk_highpass_alpha(f) (uint8_t) (255.0 * (1.0 / (2.0 * M_PI * SMP_DT * (f) + 1)))

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
int8_t sample_scale(int8_t sample, uint8_t volume) {
	return mul_fix_su8(sample, volume);
}

// finite impulse response low-pass filter
// alpha must be calculated with fir_mk_lowpass_alpha
// y1 = last output value
// x1 = last input value
// x0 = current input value
// returns y0, the current output value
int8_t fir_lowpass(int8_t y1, int8_t x1, int8_t x0, uint8_t alpha) {
	//return madd_fix_su8(last, 255 - alpha, (int16_t) current * (uint16_t) alpha);
	//return blend_fix_s8(last, current, alpha);
	return mul_fix_su8(y1, 255 - alpha) + mul_fix_su8(x0, alpha);
	//double a = alpha / 255.0;
	//return (int8_t) (y1 * (1.0 - a) + x0 * a);
}

// finite impulse response low-pass filter
// alpha must be calculated with fir_mk_highpass_alpha
// y1 = last output value
// x1 = last input value
// x0 = current input value
// returns y0, the current output value
int8_t fir_highpass(int8_t y1, int8_t x1, int8_t x0, uint8_t alpha) {
	return mul_fix_su8(y1 + x0 - x1, alpha);
	//double a = alpha / 255.0;
	//return (int8_t) (((double) y1 + x0 - x1) * a);
}

int main(int argc, char **argv) {
	//srand(0);
	lfsr_init(1);
	uint8_t *buffer = calloc(SMP_BUFFER, 1);
	
	int fd = open("out.pcm", O_CREAT | O_WRONLY | O_TRUNC, 0666);
	
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
		uint8_t alpha_low = fir_mk_lowpass_alpha(fc);
		uint8_t alpha_high = fir_mk_highpass_alpha(fc);
		printf("fc = %uHz alpha_low = %u alpha_high = %u\n", fc, alpha_low, alpha_high);
		int8_t y1 = 0, x1 = 0;
		int8_t min = 127, max = -128;
		for (size_t i = 0; i < SMP_BUFFER; i++) {
			// sample production
			int8_t r = (int8_t) rnd_gaussian();
			// volume scaling
			int8_t x0 = sample_scale(r, SMP_VOLUME);
			
			// filter depending on last input, last output and current input values
			int8_t y0 = fir_lowpass(y1, x1, x0, alpha_low);
// 			int8_t y0 = fir_highpass(y1, x1, x0, alpha_high);
			
			// stats
			if (y0 < min) min = y0;
			if (y0 > max) max = y0;

			// store the sample
			buffer[i] = (uint8_t) y0;
			// update last values
			x1 = x0;
			y1 = y0;
		}
		printf("min = %d max = %d\n", min, max);
		// frequency step (30Hz)
		fc = (fc + 50) % 10000;
// 		fc = (fc + 1000) % 20000;
		// send for playback
		pa_simple_write(
			s,
			buffer,
			SMP_BUFFER,
			NULL
		);
		write(fd, buffer, SMP_BUFFER);
	}
	
	pa_simple_drain(s, NULL);
	pa_simple_free(s);
	free(buffer);
	
	close(fd);
	
	return 0;
}
