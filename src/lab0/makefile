CC = clang -mcpu=cortex-a53 --target=aarch64-rpi3-elf
LD = ld.lld -m aarch64elf
OBJCPY = llvm-objcopy --output-target=aarch64-rpi3-elf

CFLAGS = -Wall

.PHONY: all clean run

all: kernel8.img clean

lab0.o: lab0.s
	$(CC) $(CFLAGS) -c lab0.s -o lab0.o

kernel8.img: lab0.o
	$(LD) -T linker.ld -o kernel8.elf lab0.o
	$(OBJCPY) -O binary kernel8.elf kernel8.img

clean:
	rm -f *.o

run:
	qemu-system-aarch64 -M raspi3ap -kernel kernel8.img -display none -d in_asm

