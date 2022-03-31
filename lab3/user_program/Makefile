SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
ASMS = $(wildcard *.S)
ASM_OBJS = $(ASMS:.S=.o)
INCLUDE = ../lib
CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -I$(INCLUDE)
CC = clang --target=aarch64-elf -mcpu=cortex-a53+nosimd
LIB = ../lib/lib.a

prog.img: prog.bin insert_addr.sh
	./insert_addr.sh 0x150000 ./prog.bin ./prog.img

prog.bin: $(LIB) $(OBJS) $(ASM_OBJS)
	ld.lld -m aarch64elf -nostdlib -T link.ld $^ -o prog.elf 
	llvm-objcopy -O binary prog.elf prog.bin

start.o: start.S
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean run

clean: 
	rm -f *.img *.o *.bin *.elf
