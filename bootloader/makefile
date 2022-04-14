CROSS = aarch64-unknown-linux-gnu
CC = $(CROSS)-gcc
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
LD = $(CROSS)-ld
OBJCPY = $(CROSS)-objcopy

INCLUDE = -I ./include
SRCS = $(wildcard ./*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

all: clean bootloader.img

boot.o: boot.s
	$(CC) $(CFLAGS) $(INCLUDE) -c boot.s -o boot.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

bootloader.img: boot.o $(OBJS)
	$(LD) -nostdlib -nostartfiles boot.o $(OBJS) -T linker.ld -o bootloader.elf
	$(OBJCPY) -O binary bootloader.elf bootloader.img

clean:
	rm bootloader.elf bootloader.img *.o >/dev/null 2>/dev/null || true
