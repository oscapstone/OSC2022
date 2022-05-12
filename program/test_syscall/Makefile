CROSS_COMPILE = aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

.PHONY: clean

CFLAGS := -O0 -fno-stack-protector -ffreestanding -fdata-sections -ffunction-sections -ggdb

test_syscall: start.S.o main.c.o
	$(LD) -T linker.ld $^ -o test_syscall.elf
	$(CROSS_COMPILE)objcopy -O binary test_syscall.elf test_syscall
	cp $@ ../../rootfs

start.S.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.S.o

main.c.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.c.o

clean:
	rm test_syscall.elf test_syscall start.S.o main.c.o