TOOLCHAIN_PREFIX = aarch64-unknown-linux-gnu-
CC = $(TOOLCHAIN_PREFIX)gcc
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
LD = $(TOOLCHAIN_PREFIX)ld
LINK_SCRIPT = linker.ld
OBJCPY = $(TOOLCHAIN_PREFIX)objcopy

INCLUDE = -I ./include/
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

all: clean kernel8.img

start.o: start.s
	$(CC) $(CFLAGS) $(INCLUDE) -c start.s -o start.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

kernel8.img: start.o $(OBJS)
	$(LD) -nostdlib -nostartfiles start.o $(OBJS) -T $(LINK_SCRIPT) -o kernel8.elf
	$(OBJCPY) -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf kernel8.img *.o src/*.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio