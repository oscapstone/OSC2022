ARMGNU = aarch64-linux-gnu
CC = $(ARMGNU)-gcc
LK = $(ARMGNU)-ld
OBJCPY = $(ARMGNU)-objcopy
QEMU = qemu-system-aarch64

A_SRCS = $(wildcard kernel/lib/*.S)
C_SRCS = $(wildcard kernel/lib/*.c)
OBJS = $(A_SRCS:.S=.o) $(C_SRCS:.c=.o)
INCLUDE = kernel/include
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -I$(INCLUDE)
CPIO_FILE = initramfs.cpio
DTB_FILE = bcm2710-rpi-3-b-plus.dtb
IMAGE = kernel8

.PHONY: clean

all: $(IMAGE).img

$(IMAGE).img: kernel8.elf
	$(OBJCPY) -O binary $(IMAGE).elf kernel8.img

$(IMAGE).elf: $(OBJS)
	$(LK) $(OBJS) -T kernel/linker.ld -o $(IMAGE).elf

kernel/%.o: kernel/lib/%.S
	$(CC) -c $< $(CFLAGS) -o $@

kernel/%.o: kernel/lib/%.c
	$(CC) -c $< $(CFLAGS) -o $@

run: $(IMAGE).img
	$(QEMU) -M raspi3 -kernel $(IMAGE).img -initrd $(CPIO_FILE) -dtb $(DTB_FILE) -display none -serial null -serial stdio

clean:
	rm -rf $(OBJS) *.img *.elf


