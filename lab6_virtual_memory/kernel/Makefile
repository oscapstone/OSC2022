BUILD = ./build
LIB_DIR = ../../lib
LIB_BUILD_DIR = ../../lib/build
INC_DIR = ../../include

SRCS = $(wildcard *.c)
LIBS = $(notdir $(wildcard $(LIB_DIR)/*.c))

OBJS = $(patsubst %.c, $(BUILD)/%.o, $(SRCS)) $(patsubst %.c, $(LIB_BUILD_DIR)/%.o, $(LIBS))
CFLAGS = -Wall -nostdinc -nostdlib -nostartfiles -ffreestanding -I $(INC_DIR)

all: img

$(BUILD)/start.o: start.S
	aarch64-linux-gnu-gcc $(CFLAGS) -MMD -c $< -o $@

$(BUILD)/exception_table.o: exception_table.S
	aarch64-linux-gnu-gcc $(CFLAGS) -MMD -c $< -o $@

$(BUILD)/switch.o: switch.S
	aarch64-linux-gnu-gcc $(CFLAGS) -MMD -c $< -o $@

$(BUILD)/%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) -MMD -c $< -o $@

$(LIB_BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	aarch64-linux-gnu-gcc $(CFLAGS) -MMD -c $< -o $@

img: $(BUILD)/start.o $(BUILD)/exception_table.o $(BUILD)/switch.o $(OBJS)
	aarch64-linux-gnu-ld $(BUILD)/start.o $(BUILD)/exception_table.o $(BUILD)/switch.o $(OBJS) -T linker.ld -o $(BUILD)/kernel8.elf
	aarch64-linux-gnu-objcopy -O binary $(BUILD)/kernel8.elf $(BUILD)/kernel8.img

clean:
	rm $(BUILD)/*

clean_lib:
	rm $(LIB_BUILD_DIR)/*.o

run:
	qemu-system-aarch64 -M raspi3b -kernel $(BUILD)/kernel8.img -serial null -serial stdio -initrd ../user_program/initramfs.cpio -dtb ../../files_tools/bcm2710-rpi-3-b-plus.dtb

run_debug:
	qemu-system-aarch64 -M raspi3b -kernel $(BUILD)/kernel8.img -d in_asm -serial null -serial pty -initrd ../user_program/initramfs.cpio -dtb ../../files_tools/bcm2710-rpi-3-b-plus.dtb

run_gdb:
	qemu-system-aarch64 -M raspi3b -kernel $(BUILD)/kernel8.img -display none -serial null -serial pty -S -s -initrd ../user_program/initramfs.cpio -dtb ../../files_tools/bcm2710-rpi-3-b-plus.dtb&
	gdb-multiarch
dump:
	aarch64-linux-gnu-objdump -D -z $(BUILD)/kernel8.elf

firm:
	dd if=/dev/zero of=$(BUILD)/lab.img count=50 bs=1M
	(echo o; echo n; echo p; echo 1; echo 2048; echo 102399; echo t; echo c; echo w) | fdisk $(BUILD)/lab.img
	mkfs.vfat $(BUILD)/lab.img
	sudo mkdir /mnt/lab
	sudo mount $(BUILD)/lab.img /mnt/lab
	sudo cp ../../firmware/* /mnt/lab
	sudo cp $(BUILD)/kernel8.img /mnt/lab
	sudo cp ../user_program/initramfs.cpio ./config.txt ../../files_tools/bcm2710-rpi-3-b-plus.dtb /mnt/lab
	sudo umount /mnt/lab
	sudo rmdir /mnt/lab
