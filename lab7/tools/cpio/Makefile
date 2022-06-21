CC = aarch64-linux-gnu-gcc
CPP = aarch64-linux-gnu-cpp
override CFLAGS += \
        -nostdlib \
        -nodefaultlibs \
        -ffreestanding \
        -nostartfiles \
        -mgeneral-regs-only \
        -fno-builtin \

LDFLAGS ?= \
        -nostdlib \

CXXFLAGS =
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
OBJDUMP = aarch64-linux-gnu-objdump
QEMU = qemu-system-aarch64
RASPI_VER = raspi3b


LINKER_FILE = ./linker.ld
HEADERS = $(PWD)/include/
OUTPUT = $(PWD)/build
KNAME = kernel8
BNAME = bootloader

.PHONY: all  


all:  
	$(CC) -c -o test.o test.S $(CFLAGS)
	$(LD) -T linker.ld -o test.elf test.o $(LDFLAGS)
	$(OBJCOPY) -O binary test.elf test.img 

