CROSS_COMPILE = aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

NAME = "test_el0_timer"

.PHONY: clean

CFLAGS := -O0 -fno-stack-protector -ffreestanding -fdata-sections -ffunction-sections

$(NAME): $(NAME).S.o
	$(LD) -T linker.ld $^ -o $(NAME).elf
	$(CROSS_COMPILE)objcopy -O binary $(NAME).elf $(NAME)
	cp $@ ../../rootfs

$(NAME).S.o: start.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(NAME).elf $(NAME) $(NAME).S.o