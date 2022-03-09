.PHONY: all clean qemu on-board

ARMGNU ?= aarch64-linux-gnu

FLAGS = -O0 -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
ASMOPS = -Iinclude

BUILD_DIR = build
SRC_DIR = src

all: kernel8.img

kernel8.img: $(SRC_DIR)/*.S $(SRC_DIR)/*.c
	$(ARMGNU)-gcc $(FLAGS) -T$(SRC_DIR)/linker.ld $^ -o kernel8.elf -I.
	$(ARMGNU)-objcopy -O binary kernel8.elf kernel8.img

qemu:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio

on-board:
	sudo screen /dev/ttyUSB0 115200

clean:
	rm -f *.elf *.img