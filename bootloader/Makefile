C_SRCS = $(wildcard *.c)
C_OBJS = $(C_SRCS:.c=_c.o)
S_SRCS = $(wildcard *.S)
S_OBJS = $(S_SRCS:.S=_s.o)

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
CC = aarch64-linux-gnu-gcc
LINKER = aarch64-linux-gnu-ld
OBJ_CPY = aarch64-linux-gnu-objcopy

all: clean kernel8.img

%_s.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%_c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


kernel8.img: $(S_OBJS) $(C_OBJS)
	$(LINKER) -nostdlib -nostartfiles $(S_OBJS) $(C_OBJS) -T link.ld -o kernel8.elf
	$(OBJ_CPY) -O binary kernel8.elf kernel8.img
clean:
	rm -rf *.img *.o *.elf
	
run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

run_lab2:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial pty

gdb:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -S -s -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -serial null -serial stdio



