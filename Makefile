ARMGNU ?= aarch64-linux-gnu

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -g -O0

DEVPATH ?= /dev/ttyUSB0
SDCPATH ?= /dev/sdc1

all: kernel8.img clean

start.o: start.S
	$(ARMGNU)-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@

kernel8.img: start.o $(OBJS)
	$(ARMGNU)-ld -nostdlib -nostartfiles start.o $(OBJS) -T link.ld -o kernel8.elf
	$(ARMGNU)-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm *.o >/dev/null 2>/dev/null || true

cpio:
	cd rootfs && find . | cpio -o -H newc > ../initramfs.cpio

user:
	rm initramfs.cpio
	wget https://oscapstone.github.io/_downloads/58c515e3041658a045033c8e56ecff4c/initramfs.cpio

run: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

rune: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int

rund: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

gdb: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -S -s

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
