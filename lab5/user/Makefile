CFLAGS ?= \
        -nostdlib \
        -nodefaultlibs \
        -ffreestanding \
        -nostartfiles \
        -mgeneral-regs-only \
        -fno-builtin \
		-g

LDFLAGS ?= \
        -nostdlib \
		-g

LD = aarch64-linux-gnu-ld
CC = aarch64-linux-gnu-gcc
CPP = aarch64-linux-gnu-cpp
OBJCOPY = aarch64-linux-gnu-objcopy

all: 
	$(CC) -c start.S -o start.o -I. $(CFLAGS)
	$(CC) -c user_lib.c -o user_lib.o -I. $(CFLAGS)
	$(CC) -c test.c -o test.o -I. $(CFLAGS)
	$(LD) -T linker.ld -o test.elf start.o test.o user_lib.o $(LDFLAGS)
	$(OBJCOPY) -O binary test.elf syscall.img


clean:
	rm test.o user_lib.o syscall.img start.o
