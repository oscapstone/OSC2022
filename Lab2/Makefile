ARMGNU = aarch64-elf

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector
INCLUDES = -Iinclude

KERNEL_SRC_DIR = kernel
BOOT_SRC_DIR = bootloader
LIB_SRC_DIR = src
OBJ_DIR = objs

RAMFS = initramfs.cpio

.PHONY: clean all $(RAMFS)

##############
_KERNEL_C_FILES   = $(wildcard $(KERNEL_SRC_DIR)/*.c)
_KERNEL_ASM_FILES = $(wildcard $(KERNEL_SRC_DIR)/*.S)
KERNEL_OBJ_FILES  = $(_KERNEL_C_FILES:$(KERNEL_SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)
KERNEL_OBJ_FILES += $(_KERNEL_ASM_FILES:$(KERNEL_SRC_DIR)/%.S=$(OBJ_DIR)/%_s.o)
##############
_BOOT_C_FILES   = $(wildcard $(BOOT_SRC_DIR)/*.c)
_BOOT_ASM_FILES = $(wildcard $(BOOT_SRC_DIR)/*.S)
BOOT_OBJ_FILES  = $(_BOOT_C_FILES:$(BOOT_SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)
BOOT_OBJ_FILES += $(_BOOT_ASM_FILES:$(BOOT_SRC_DIR)/%.S=$(OBJ_DIR)/%_s.o)
##############
_LIB_C_FILES   = $(wildcard $(LIB_SRC_DIR)/*.c)
_LIB_ASM_FILES = $(wildcard $(LIB_SRC_DIR)/*.S)
LIB_OBJ_FILES  = $(_LIB_C_FILES:$(LIB_SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)
LIB_OBJ_FILES += $(_LIB_ASM_FILES:$(LIB_SRC_DIR)/%.S=$(OBJ_DIR)/%_s.o)
##############

############## compile kernel ##############
$(OBJ_DIR)/%_c.o: $(KERNEL_SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(CFLAGS) $(INCLUDES) -MMD -c $< -o $@

$(OBJ_DIR)/%_s.o: $(KERNEL_SRC_DIR)/%.S
	$(ARMGNU)-gcc $(CFLAGS) -MMD -c $< -o $@
############ compile bootloader ############	
$(OBJ_DIR)/%_c.o: $(BOOT_SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(CFLAGS) $(INCLUDES) -MMD -c $< -o $@

$(OBJ_DIR)/%_s.o: $(BOOT_SRC_DIR)/%.S
	$(ARMGNU)-gcc $(INCLUDES) -MMD -c $< -o $@

############# compile library #############	
$(OBJ_DIR)/%_c.o: $(LIB_SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(INCLUDES) $(CFLAGS) -MMD -c $< -o $@

$(OBJ_DIR)/%_s.o: $(LIB_SRC_DIR)/%.S
	$(ARMGNU)-gcc $(INCLUDES) $(CFLAGS) -MMD -c $< -o $@

all: clean kernel8.img bootloader.img $(RAMFS)

bootloader.img: $(BOOT_SRC_DIR)/bootloader.ld $(BOOT_OBJ_FILES) $(LIB_OBJ_FILES)
	$(ARMGNU)-ld -T $< -o $(OBJ_DIR)/bootloader.elf $(BOOT_OBJ_FILES) $(LIB_OBJ_FILES)
	$(ARMGNU)-objcopy $(OBJ_DIR)/bootloader.elf -O binary $@

kernel8.img: $(KERNEL_SRC_DIR)/kernel.ld $(KERNEL_OBJ_FILES) $(LIB_OBJ_FILES)
	$(ARMGNU)-ld -nostdlib -nostartfiles -T $< -o $(OBJ_DIR)/kernel8.elf $(KERNEL_OBJ_FILES) $(LIB_OBJ_FILES)
	$(ARMGNU)-objcopy $(OBJ_DIR)/kernel8.elf -O binary $@

$(RAMFS): rootfs
	cd ./rootfs; pwd; find . | cpio -o -H newc > ../$@; cd ..;
clean:
	rm -rf $(OBJ_DIR)/* *.img *.cpio
run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img  -serial null -serial stdio -initrd initramfs.cpio
test_makefile:
	echo $(OBJS)
