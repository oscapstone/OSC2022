.PHONY: all clean qemu-kernel qemu-bootloader test-bootloader on-board load-kernel qemu

ARMGNU ?= aarch64-linux-gnu

FLAGS = -O0 -Wall -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only -g -fPIC
INC = include
LIB = lib
CPIO = archive/initramfs.cpio
USER_PROG = timer_test

all: kernel8.img bootloader.img $(CPIO) $(USER_PROG).img

kernel8.img: kernel/*.S kernel/*.c $(LIB)/*.S $(LIB)/*.c rd.o
	$(ARMGNU)-gcc $(FLAGS) -Tkernel/linker.ld $^ -o kernel8.elf -I$(INC)
	$(ARMGNU)-objcopy -O binary kernel8.elf kernel8.img

bootloader.img: bootloader/*.S bootloader/*.c $(LIB)/*.S $(LIB)/mini_uart.c $(LIB)/utils.c
	$(ARMGNU)-gcc $(FLAGS) -Tbootloader/linker.ld $^ -o bootloader.elf -I$(INC)
	$(ARMGNU)-objcopy -O binary bootloader.elf bootloader.img

$(USER_PROG).img: user/$(USER_PROG).c $(LIB)/*.c $(LIB)/*.S
	$(ARMGNU)-gcc $(FLAGS) -Tuser/linker.ld $^ -o $(USER_PROG).elf
	$(ARMGNU)-objcopy -O binary $(USER_PROG).elf $(USER_PROG).img

$(CPIO): $(USER_PROG).img
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