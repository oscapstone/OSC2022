ARMGNU ?= aarch64-linux-gnu

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

DEVPATH ?= /dev/ttyUSB0
SDCPATH ?= /dev/sdc1

all: kernel8.img clean

start.o: start.S
	$(ARMGNU)-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	$(ARMGNU)-gcc $(CFLAGS) -c -g $< -o $@

kernel8.img: start.o $(OBJS)
	$(ARMGNU)-ld -nostdlib -nostartfiles start.o $(OBJS) -T link.ld -o kernel8.elf
	$(ARMGNU)-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm *.o >/dev/null 2>/dev/null || true

cpio:
	cd rootfs && find . | cpio -o -H newc > ../initramfs.cpio

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

rune:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int

gdb:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int -S -s

dd:
	sudo mount $(SDCPATH) /media
	sudo cp config.txt /media/config.txt
	sudo cp initramfs.cpio /media/initramfs.cpio
	sudo cp bcm2710-rpi-3-b-plus.dtb /media/bcm2710-rpi-3-b-plus.dtb
	sudo cp kernel8.img /media/kernel8.img
	sudo umount $(SDCPATH)

send:
	sudo sh -c 'echo > $(DEVPATH)'
	sudo sh -c 'ksize=`stat -c %s ./kernel8.img` && printf "%07d" $$ksize > $(DEVPATH)'
	sudo sh -c 'cat ./kernel8.img > $(DEVPATH)'

screen:
	sudo screen $(DEVPATH) 115200
