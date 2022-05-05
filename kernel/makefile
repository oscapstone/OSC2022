CROSS = aarch64-unknown-linux-gnu
CC = $(CROSS)-gcc
CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles -g
LD = $(CROSS)-ld
OBJCPY = $(CROSS)-objcopy

INCLUDE = -I ../include
SRCS = $(wildcard ./*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))
SRCS_ASM = $(wildcard ./*.s)
OBJS_ASM = $(patsubst %.s, %.o, $(SRCS_ASM))

all: clean kernel8.img

%.o: %.s
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

kernel8.img: $(OBJS) $(OBJS_ASM)
	$(LD) -nostdlib -nostartfiles $(OBJS) $(OBJS_ASM) -T linker.ld -o kernel8.elf
	$(OBJCPY) -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf kernel8.img *.o >/dev/null 2>/dev/null || true
