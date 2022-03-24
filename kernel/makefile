CROSS = aarch64-unknown-linux-gnu
CC = $(CROSS)-gcc
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
LD = $(CROSS)-ld
OBJCPY = $(CROSS)-objcopy

INCLUDE = -I ../include
SRCS = $(wildcard ./*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

all: clean kernel8.img

boot.o: boot.s
	$(CC) $(CFLAGS) $(INCLUDE) -c boot.s -o boot.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

kernel8.img: boot.o $(OBJS)
	$(LD) -nostdlib -nostartfiles boot.o $(OBJS) -T linker.ld -o kernel8.elf
	$(OBJCPY) -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf kernel8.img *.o >/dev/null 2>/dev/null || true
