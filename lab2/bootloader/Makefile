C_SRCS = $(wildcard *.c)
C_OBJS = $(C_SRCS:.c=_c.o)
S_SRCS = $(wildcard *.S)
S_OBJS = $(S_SRCS:.S=_s.o)

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
CC = aarch64-linux-gnu-gcc
LINKER = aarch64-linux-gnu-ld
OBJ_CPY = aarch64-linux-gnu-objcopy

all: clean bootloader.img

%_s.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%_c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


bootloader.img: $(S_OBJS) $(C_OBJS)
	$(LINKER) -nostdlib -nostartfiles $(S_OBJS) $(C_OBJS) -T link.ld -o kernel8.elf
	$(OBJ_CPY) -O binary kernel8.elf bootloader.img
clean:
	rm -rf *.img *.o *.elf
	
run:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -serial null -serial stdio

run_lab2:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -serial null -serial pty



