SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
INCLUDE = ../lib
CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd -I$(INCLUDE)
CC = clang --target=aarch64-elf
LIB = ../lib/lib.a

kernel8.img: start.o $(OBJS) $(LIB)
	ld.lld -m aarch64elf -nostdlib -T link.ld $^ -o kernel8.elf 
	llvm-objcopy -O binary kernel8.elf kernel8.img

start.o: start.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB):
	make -C $(LIBDIR)

clean: 
	rm -f kernel8.elf *.o kernel8.img

run: kernel8.img
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial pty

debug: kernel8.img
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial pty -S -s
