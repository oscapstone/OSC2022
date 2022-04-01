CROSS_COMPILE = aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

.PHONY: clean

CFLAGS := -O0 -fno-stack-protector -ffreestanding -fdata-sections -ffunction-sections

test_el0_exception: test_el0_exception.S.o
	$(LD) -T linker.ld $^ -o test_el0_exception.elf
	$(CROSS_COMPILE)objcopy -O binary test_el0_exception.elf test_el0_exception
	cp $@ ../../rootfs

test_el0_exception.S.o: start.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm test_el0_exception.elf test_el0_exception test_el0_exception.S.o