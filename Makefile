shell = /bin/bash
CROSS_COMPILE = aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
BUILD_DIR := build
SRC_DIR := src
KERNEL_DIR := $(SRC_DIR)/kernel
BOOTLOADER_DIR := $(SRC_DIR)/bootloader
LIB_DIR := $(SRC_DIR)/lib

KERNEL_SRCS := $(shell cd src; find kernel lib -name '*.[cS]')
KERNEL_OBJS := $(KERNEL_SRCS:%=$(BUILD_DIR)/%.o)

BOOTLOADER_SRCS := $(shell cd src; find bootloader -name '*.[cS]')
BOOTLOADER_OBJS := $(BOOTLOADER_SRCS:%=$(BUILD_DIR)/%.o)

CFLAGS := -O0 -I $(SRC_DIR)/include -I $(KERNEL_DIR)/include -fno-stack-protector -ffreestanding -fdata-sections -ffunction-sections -ggdb

.PHONY: clean all rootfs

all: kernel8.img bootloader.img
	cp kernel8.img rpi3/
	cp bootloader.img rpi3/

clean:
	rm -rf build kernel8.img kernel8.elf bootloader.elf bootloader.img

rootfs:
	cd rootfs && find . | cpio -o -H newc > ../initramfs.cpio
	cp initramfs.cpio rpi3/

kernel8.img: $(KERNEL_OBJS)
	echo $(KERNEL_SRCS)
	echo $(KERNEL_OBJS)
	$(LD) -T $(KERNEL_DIR)/linker.ld $^ -o kernel8.elf
	$(CROSS_COMPILE)objcopy -O binary kernel8.elf kernel8.img

bootloader.img: $(BOOTLOADER_OBJS)
	echo $(BOOTLOADER_SRCS)
	echo $(BOOTLOADER_OBJS)
	$(LD) -T $(BOOTLOADER_DIR)/linker.ld $^ --gc-sections -o bootloader.elf
	$(CROSS_COMPILE)objcopy -O binary bootloader.elf bootloader.img

$(BUILD_DIR)/kernel/%.c.o: $(KERNEL_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.S.o: $(KERNEL_DIR)/%.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/bootloader/%.c.o: $(BOOTLOADER_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DBOOTLOADER -c $< -o $@

$(BUILD_DIR)/bootloader/%.S.o: $(BOOTLOADER_DIR)/%.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DBOOTLOADER -c $< -o $@

$(BUILD_DIR)/lib/%.c.o: $(LIB_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.S.o: $(LIB_DIR)/%.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

