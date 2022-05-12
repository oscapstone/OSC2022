CROSS_COMPILE = aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

.PHONY: clean

CFLAGS := -O0 -fno-stack-protector -ffreestanding -fdata-sections -ffunction-sections -ggdb

test_loop: start.S.o main.c.o
	$(LD) -T linker.ld $^ -o test_loop.elf
	$(CROSS_COMPILE)objcopy -O binary test_loop.elf test_loop
	cp $@ ../../rootfs

start.S.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.S.o

main.c.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.c.o

clean:
	rm test_loop.elf test_loop start.S.o main.c.o