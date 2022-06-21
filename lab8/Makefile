ASM_SRCS = $(wildcard ./src/*.S)
C_SRCS = $(wildcard ./src/*.c)
OBJS = $(C_SRCS:.c=.o) $(ASM_SRCS:.S=.o)

INCLUDE = ./lib/ ./include/
INCLUDE_OPT = $(foreach d, $(INCLUDE), -I$d)

CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib $(INCLUDE_OPT) 
CC = clang --target=aarch64-elf -mcpu=cortex-a53+nosimd -mgeneral-regs-only

LIB_SRCS = $(wildcard ./lib/*.c)
LIB_OBJS = $(LIB_SRCS:.c=.o)
LIB = ./lib/lib.a
AR = llvm-ar

kernel8.img: kernel8.bin insert_size.sh
	./insert_size.sh ./kernel8.bin ./kernel8.img

kernel8.bin: $(LIB) $(OBJS)
	ld.lld -m aarch64elf -nostdlib -T src/link.ld $^ -o kernel8.elf 
	llvm-objcopy -O binary kernel8.elf kernel8.bin

$(LIB): $(LIB_OBJS)
	$(AR) r $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run debug
clean:
	rm -f kernel8.img src/*.o *.bin *.elf lib/*.o lib/*.a

run: kernel8.bin
	cp sfn_nctuos.img.bak sfn_nctuos.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.bin -initrd initramfs.cpio -serial null -serial stdio  -dtb bcm2710-rpi-3-b-plus.dtb -drive if=sd,file=sfn_nctuos.img,format=raw

debug: kernel8.bin
	cp sfn_nctuos.img.bak sfn_nctuos.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.bin -initrd initramfs.cpio -serial null -serial stdio  -dtb bcm2710-rpi-3-b-plus.dtb -drive if=sd,file=sfn_nctuos.img,format=raw -S -s
