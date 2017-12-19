PRG            = wildpixel
OBJ            = ws2812.o main.o palette.o lfsr.o tinymath.o
OPTIMIZE       = -Os -fno-move-loop-invariants -fno-tree-scev-cprop -fno-inline-small-functions -fdata-sections -flto -ffunction-sections #-fwhole-program
PROGRAMMER     = arduino -P /dev/ttyUSB0 -b 57600

MCU_TARGET     = atmega328p
DEFS           = -DF_CPU=16000000UL

LIBS           =

CC             = avr-gcc
DUDE           = avrdude
SIZE           = avr-size

CFLAGS         = -g -Wall -std=gnu99 $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
LDFLAGS        = $(OPTIMIZE) -Wl,-Map,$(PRG).map

DUDEFLAGS      = -c $(PROGRAMMER) -p $(MCU_TARGET)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: $(PRG).hex $(PRG).lst

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(SIZE) --format=avr --mcu=$(MCU_TARGET) $@

clean:
	rm -rf *.o *.elf *.lst *.map *.hex *.bin *.srec $(EXTRA_CLEAN_FILES)

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

clocks.h: genclocks.pl
	./genclocks.pl

flashall: $(PRG).hex $(PRG)_eeprom.hex
	$(DUDE) $(DUDEFLAGS) -U eeprom:w:$(PRG)_eeprom.hex:i -U flash:w:$(PRG).hex:i

flash: $(PRG).hex
	$(DUDE) $(DUDEFLAGS) -U flash:w:$(PRG).hex:i

eeprom: $(PRG)_eeprom.hex
	$(DUDE) $(DUDEFLAGS) -U eeprom:w:$(PRG)_eeprom.hex:i

#fuse:
#	$(DUDE) $(DUDEFLAGS) -U hfuse:w:0xd9:m -U lfuse:w:0x24:m

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@ \
	|| { echo empty $@ not generated; exit 0; }

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@ \
	|| { echo empty $@ not generated; exit 0; }

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@ \
	|| { echo empty $@ not generated; exit 0; }
