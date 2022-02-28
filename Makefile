shell = /bin/bash
CROSS_COMPILE = aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
BUILD_DIR := build
SRC_DIR := src
KERNEL_DIR := $(SRC_DIR)/kernel
LIB_DIR := $(SRC_DIR)/lib
KERNEL_SRCS := $(shell cd src; find kernel lib -name '*.[cS]')
KERNEL_OBJS := $(KERNEL_SRCS:%=$(BUILD_DIR)/%.o)

CFLAGS := -O0 -I $(SRC_DIR)/include -fno-stack-protector -ffreestanding

.PHONY: clean all

all: kernel8.img

clean:
	rm -rf build kernel8.img kernel8.elf

kernel8.img: $(KERNEL_OBJS)
	echo $(KERNEL_SRCS)
	echo $(KERNEL_OBJS)
	$(LD) -T $(KERNEL_DIR)/linker.ld $^ -o kernel8.elf
	$(CROSS_COMPILE)objcopy -O binary kernel8.elf kernel8.img

$(BUILD_DIR)/kernel/%.c.o: $(KERNEL_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.S.o: $(KERNEL_DIR)/%.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.c.o: $(LIB_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.S.o: $(LIB_DIR)/%.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

