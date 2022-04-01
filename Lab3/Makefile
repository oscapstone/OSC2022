.PHONY: all clean qemu-kernel qemu-bootloader test-bootloader on-board load-kernel qemu

ARMGNU ?= aarch64-linux-gnu

FLAGS = -O0 -Wall -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only -g
INC = include
LIB = lib
CPIO = archive/initramfs.cpio

all: kernel8.img bootloader.img $(CPIO) user.img 

kernel8.img: kernel/*.S kernel/*.c $(LIB)/*.S $(LIB)/*.c rd.o
	$(ARMGNU)-gcc $(FLAGS) -Tkernel/linker.ld $^ -o kernel8.elf -I$(INC)
	$(ARMGNU)-objcopy -O binary kernel8.elf kernel8.img

bootloader.img: bootloader/*.S bootloader/*.c $(LIB)/*.S $(LIB)/*.c
	$(ARMGNU)-gcc $(FLAGS) -Tbootloader/linker.ld $^ -o bootloader.elf -I$(INC)
	$(ARMGNU)-objcopy -O binary bootloader.elf bootloader.img

user.img: user/*.S
	$(ARMGNU)-gcc $(FLAGS) -Tuser/linker.ld $^ -o user.elf
	$(ARMGNU)-objcopy -O binary user.elf user.img

$(CPIO): user.img
	cd archive/rootfs && find . | cpio -o -H newc > ../initramfs.cpio

rd.o:$(CPIO)
	aarch64-linux-gnu-ld -r -b binary -o rd.o $(CPIO)

qemu-kernel: $(CPIO)
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd $(CPIO)

qemu-debug: $(CPIO)
	aarch64-linux-gnu-objdump -d kernel8.elf > kernel8.S
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd $(CPIO) -d in_asm

qemu-bootloader: $(CPIO)
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -display none -serial null -serial pty -initrd $(CPIO)

load-kernel:
	sudo ./load_kernel.py

qemu:
	sudo screen /dev/pts/2 115200

on-board:
	sudo screen /dev/ttyUSB0 115200

clean:
	rm -f *.elf *.img *.cpio *.o archive/*.cpio *.S *.s