# wildpixel

let your imagination run wild!

## what is it?

wildpixel is a collection of small libraries for writing fancy demos and intros
for tiny-scale computers aka microcontrollers.

## supported hardware

due to wide and easy availability, the target platform for the current
incarnation are arduino boards based on atmel microcontrollers.

some supported examples:
* arduino uno
* arduino pro mini
* numerous clones

## build and use

you need gnu make, avr-gcc, avr-libc and avrdude to compile and flash
the source code to your arduino. no arduino ide or libraries are required.

just use a simple text editor to modify the source code!
it's in plain c and makes liberal use of macros. some inline assembly is used
where necessary.

you also (obviously) need hardware to run this on.

i recommend an arduino pro mini, a couple of ws2812 leds on a stripe,
a 5v power bank and a serial programmer. also some leads and a usb a type plug
to connect the stuff together.
all of this stuff can be had dead cheap on aliexpress, for example.

### wiring

connect the input data pin of the led stripe to portd2 (yes, that's digital
pin 2 on the arduino), vcc to +5v and gnd to gnd.

the stripe should have 20 leds, arranged in 4 rows by 5 leds each.
arrange all data in pins to the left and connect the last pixel of each row
to the first led of the next.
if you want more leds, adapt config.h and main.c accordingly.

attach the programmer to the arduino. if you use the uno, you don't need this
as the usb programming interface is built in.

to make it run from a power bank later, solder two leads to the base usb plug
and fixate them with some hot glue.
these should be connected to +5v and gnd later (once it's programmed).

todo: pictures!

### compile and run

compile with:

   make

to write the resulting binary to your arduino, use:

   make flash

## modules

### ws2812

led chain module: string together a bunch of these bugs and build yourself
a display.

eventually, the module is going to support multiple chains in parallel to
support higher frame rates, but this is still left todo.

the built-in frame buffer is palette based to conserve memory and allow
certain special use cases.

### palette

a palette generator.

can use a ROM palette (needs to be pregenerated), build a palette on
startup or calculate color values on the fly.

### lfsr

avr-libc already includes a pseudo random number generator, but it uses
too much code space and too many cycles.

this one is based on a really primitive linear feedback shift register.
but it should be sufficient for many use cases.

it seems to be buggy currently. patches welcome.

### tinymath

a collection of small and/or fast math routines that can be useful if you
don't need/want accurate results.

## who & legal

wildpixel was written by and is copyright 2017 by onitake <onitake@gmail.com> .

the code is available under the simplified (2-clause) BSD license.
see the LICENSE file for details.
