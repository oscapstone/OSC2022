ARMGNU = aarch64-linux-gnu
CC = $(ARMGNU)-gcc
LK = $(ARMGNU)-ld
OBJCPY = $(ARMGNU)-objcopy
QEMU = qemu-system-aarch64

A_SRCS = $(wildcard src/lib/*.S)
C_SRCS = $(wildcard src/lib/*.c)
OBJS = $(A_SRCS:.S=.o) $(C_SRCS:.c=.o)
INCLUDE = src/include
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -I$(INCLUDE)

.PHONY: clean

all: kernel8.img

kernel8.img: kernel8.elf
	$(OBJCPY) -O binary kernel8.elf kernel8.img

kernel8.elf: $(OBJS)
	$(LK) $(OBJS) -T src/linker.ld -o kernel8.elf

src/%.o: src/lib/%.S
	$(CC) -c $< $(CFLAGS) -o $@

src/%.o: src/lib/%.c
	$(CC) -c $< $(CFLAGS) -o $@

run:
	$(QEMU) -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio

clean:
	rm -rf $(OBJS) *.img *.elf


