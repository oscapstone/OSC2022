
CROSS_COMPILER_PREFIX = aarch64-linux-gnu
SRCS = $(wildcard lib/*.c) main.c
OBJS = $(SRCS:.c=.o)

CFLAGS = -Wall -ffreestanding -nostdlib -nostartfiles -ggdb# -nostdinc

all: clean kernel8.img

start.o: start.S
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	$(CROSS_COMPILER_PREFIX)-gcc $(CFLAGS) -I ./lib/include -c $< -o $@

kernel8.img: start.o $(OBJS)
	$(CROSS_COMPILER_PREFIX)-ld -nostdlib start.o $(OBJS) -T link.ld -o kernel8.elf
	$(CROSS_COMPILER_PREFIX)-objcopy -O binary kernel8.elf kernel8.img
	$(CROSS_COMPILER_PREFIX)-size kernel8.elf

clean:
	rm kernel8.elf *.o ./lib/*.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 

debug:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -s -S -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 	

load:
	source ~/py-env/bin/activate
	python3 send_kernel.py

screen:
	screen /dev/tty.usbserial-0001 115200
