.PHONY: all clean qemu-kernel qemu-bootloader test-bootloader on-board

ARMGNU ?= aarch64-linux-gnu

FLAGS = -O0 -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
ASMOPS = -Iinclude

BUILD_DIR = build
SRC_DIR = src

all: kernel8.img bootloader.img

kernel8.img: $(SRC_DIR)/*.S $(SRC_DIR)/*.c
	$(ARMGNU)-gcc $(FLAGS) -T$(SRC_DIR)/linker.ld $^ -o kernel8.elf -I.
	$(ARMGNU)-objcopy -O binary kernel8.elf kernel8.img

bootloader.img: bootloader/*.S bootloader/*.c $(SRC_DIR)/mini_uart.c $(SRC_DIR)/utils.c $(SRC_DIR)/mm.S
	$(ARMGNU)-gcc $(FLAGS) -Tbootloader/linker.ld $^ -o bootloader.elf -I.
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