CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
CFLAGS = -mcpu=cortex-m3 -mthumb -std=c11 -Os -g3 -Wall -Wextra -Werror -ffreestanding -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables
LDFLAGS = -nostdlib -Wl,--gc-sections -Wl,-z,noexecstack -Wl,-Map=build/bai02.map -T stm32f103c8.ld
OBJECTS = build/main.o build/digits.o build/startup.o

.PHONY: all flash test clean
all: build/bai02.bin

build:
	mkdir -p build

build/main.o: main.c digits.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/digits.o: digits.c digits.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/startup.o: startup.s | build
	$(CC) -mcpu=cortex-m3 -mthumb -g3 -c $< -o $@

build/bai02.elf: $(OBJECTS) stm32f103c8.ld
	$(CC) -mcpu=cortex-m3 -mthumb $(LDFLAGS) $(OBJECTS) -lgcc -o $@
	$(SIZE) $@

build/bai02.bin: build/bai02.elf
	$(OBJCOPY) -O binary $< $@

flash: build/bai02.bin
	st-flash --reset write build/bai02.bin 0x08000000

test: | build
	gcc -std=c11 -Wall -Wextra -Werror -I. tests/test_digits.c digits.c -o build/test_digits
	./build/test_digits

clean:
	rm -rf build
