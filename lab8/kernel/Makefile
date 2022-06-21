CC = aarch64-linux-gnu-gcc
CFLAGS = -Iinclude -Wall -nostdlib -nostartfiles -ffreestanding
# ASMFLAGS = 
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
OBJCOPYFLAGS = -O binary

QEMU = qemu-system-aarch64

BUILD_DIR = build
SRC_DIR = src

all: initramfs.cpio kernel8.img

initramfs.cpio:
	cd rootfs; find . | cpio -o -H newc > ../initramfs.cpio; cd ..;

clean:
	rm $(BUILD_DIR)/* initramfs.cpio

setup_loop_device:
	sudo cp ../sfn_nctuos.img .
	qemu-img resize sfn_nctuos.img 64M
	sudo losetup -fP sfn_nctuos.img 
	sudo mount -t msdos /dev/loop23p1 image_mount
	sudo cp FAT_R.txt image_mount

remove_loop_device:
	sudo umount image_mount
	sudo losetup -d /dev/loop23 

setup_sd:
	sudo mount -t msdos /dev/ttyUSB0 sd_mount

remove_sd:
	sudo umount sd_mount

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(CC) $(ASMFLAGS) -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

kernel8.elf: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(LD) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)

kernel8.img: kernel8.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $(BUILD_DIR)/kernel8.elf $(BUILD_DIR)/kernel8.img

run:
	$(QEMU) -M raspi3 -kernel $(BUILD_DIR)/kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -drive if=sd,file=sfn_nctuos.img,format=raw

run1:
	killall -9 qemu-system-aarch64
	$(QEMU) -M raspi3 -kernel $(BUILD_DIR)/kernel8.img -display vnc=127.0.0.4:0 -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb


debug:
	killall -9 qemu-system-aarch64
	$(QEMU) -M raspi3 -kernel $(BUILD_DIR)/kernel8.img -serial null -serial stdio -display none -S -s -initrd initramfs.cpio