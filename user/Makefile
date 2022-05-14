SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
INCLUDE = -I ./include/
ASMS = a
LINK = linker
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

all: clean usr1.img run

%.o: %.S
	aarch64-linux-gnu-gcc $(CFLAGS) $(INCLUDE) -c $(ASMS).S -o $(ASMS).o -g

%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) $(INCLUDE) -c $< -o $@ -g

usr1.img: $(ASMS).o $(OBJS)
	aarch64-linux-gnu-ld -nostdlib -nostartfiles $(ASMS).o $(OBJS) -T $(LINK).ld -o usr1.elf
	aarch64-linux-gnu-objcopy -O binary usr1.elf usr1.img

clean:
	rm usr1.elf usr1.img src/*.o *.o>/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3 -kernel usr1.img --display none -serial null -serial stdio -s -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb


flash:
	sudo dd if=usr1.img of=/dev/sdb

screen:
	sudo screen /dev/ttyUSB0 115200

ptty:
	qemu-system-aarch64 -M raspi3 -kernel usr1.img -s -display none -serial null -serial pty -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

boot:
	python3 uart_boot.py
screenb:
	sudo screen /dev/pts/4 115200
cpio:
	cd rootfs
	find . | cpio -o -H newc > ../initramfs.cpio
	cd ..