.PHONY: all clean qemu-kernel qemu-bootloader test-bootloader on-board

ARMGNU ?= aarch64-linux-gnu

FLAGS = -O0 -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
INC = -Iinclude
LIB = lib

all: kernel8.img bootloader.img

kernel8.img: kernel/*.S kernel/*.c $(LIB)/*.S $(LIB)/*.c
	$(ARMGNU)-gcc $(FLAGS) -Tkernel/linker.ld $^ -o kernel8.elf -I$(INC)
	$(ARMGNU)-objcopy -O binary kernel8.elf kernel8.img

bootloader.img: bootloader/*.S bootloader/*.c $(LIB)/*.S $(LIB)/*.c
	$(ARMGNU)-gcc $(FLAGS) -Tbootloader/linker.ld $^ -o bootloader.elf -I$(INC)
	$(ARMGNU)-objcopy -O binary bootloader.elf bootloader.img

qemu-kernel:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio

qemu-bootloader:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -display none -serial null -serial pty

test-bootloader:
	./load_kernel.py
	sudo screen /dev/pts/2 115200

on-board:
	sudo screen /dev/ttyUSB0 115200

clean:
	rm -f *.elf *.img