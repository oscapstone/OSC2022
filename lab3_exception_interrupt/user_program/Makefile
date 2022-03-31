BUILD = ./build
CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles

all: img

$(BUILD)/start.o: start.s
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

img: $(BUILD)/start.o
	aarch64-linux-gnu-ld -nostdlib -nostartfiles $(BUILD)/start.o -T linker.ld -o $(BUILD)/user_program.elf
	aarch64-linux-gnu-objcopy -O binary $(BUILD)/user_program.elf $(BUILD)/user_program
	cd $(BUILD); ls user_program | cpio -o -H newc > ./initramfs.cpio

clean:
	rm $(BUILD)/*
