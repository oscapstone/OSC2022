SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
ASMS = $(wildcard *.S)
ASM_OBJS = $(ASMS:.S=.o)
INCLUDE = ../lib
CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -I$(INCLUDE)
CC = clang --target=aarch64-elf -mcpu=cortex-a53+nosimd
LIB = ../lib/lib.a

kernel8.img: kernel8.bin
	../insert_size.sh ./kernel8.bin ./kernel8.img

kernel8.bin: $(LIB) $(OBJS) $(ASM_OBJS)
	ld.lld -m aarch64elf -nostdlib -T link.ld $^ -o kernel8.elf 
	llvm-objcopy -O binary kernel8.elf kernel8.bin

start.o: start.S
	$(CC) $(CFLAGS) -c $^ -o $@

user.o: user.S
	$(CC) $(CFLAGS) -c $^ -o $@

entry.o: entry.S
	$(CC) $(CFLAGS) -c $^ -o $@

asm.o: asm.S
	$(CC) $(CFLAGS) -c $^ -o $@

$(LIB):
	make -C ../lib/

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run

clean: 
	rm -f *.img *.o *.bin *.elf

run: kernel8.bin
	qemu-system-aarch64 -M raspi3b -kernel kernel8.bin -initrd initramfs.cpio -serial null -serial stdio -display none -dtb bcm2710-rpi-3-b-plus.dtb

debug: kernel8.bin
	qemu-system-aarch64 -M raspi3b -kernel kernel8.bin -initrd initramfs.cpio -serial null -serial stdio -display none -dtb bcm2710-rpi-3-b-plus.dtb -S -s
