SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
CC = clang --target=aarch64-elf
# CC = aarch64-none-elf-gcc

all: kernel8.img

start.o: start.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@

kernel8.img: start.o $(OBJS)
	ld.lld -m aarch64elf -nostdlib -T link.ld $^ -o kernel8.elf 
	llvm-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm -f kernel8.elf *.o kernel8.img

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio

run-debug:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -S -s -serial null -serial stdio
