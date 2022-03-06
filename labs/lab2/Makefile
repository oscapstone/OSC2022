ARM := aarch64-none-linux-gnu
GCC := gcc


ROOT_PATH := ../..
SRC_DIR := ../../src
LINK_DIR := ../../linker-script



LDFILE := $(LINK_DIR)/linker.ld
C_FLAG := -g -Wall -static -nostdlib -nostartfiles -fno-builtin-memset -fno-builtin-strtol
INC := -I../../include
LD_FLAG := -static -nostdlib -nostartfiles
# -L$(ROOT_PATH)/setup

LIB_DIR := $(SRC_DIR)/main/lib
MAIN_DIR := $(SRC_DIR)/main/app
BOOTLINKER := $(LINK_DIR)/bootloader.ld

# %.S: %.c
# 	$(ARM)-gcc $(INC) $(C_FLAG) -S $< -o $@
SRC_C := $(wildcard $(LIB_DIR)/*.c )
OBJ_C := $(patsubst %.c, %.o, $(SRC_C))
SRC_S := $(wildcard $(ROOT_PATH)/setup/*.S)
OBJ_S := $(patsubst %.S, %.o, $(SRC_S))
SRC := $(SRC_C) $(SRC_S)
OBJ := $(OBJ_C) $(OBJ_S)


%.o: %.c
	$(ARM)-gcc $(INC) $(C_FLAG) -c $< -o $@

%.o: %.S
	$(ARM)-gcc $(C_FLAG) -c $< -o $@ -g

.INTERMEDIATE : $(OBJ)

%.elf: $(SRC_DIR)/main/app/%.o $(OBJ)
	$(ARM)-gcc $(LD_FLAG) -T$(LDFILE) $(INC) $^ -o $@

bootloader.elf: $(SRC_DIR)/main/bootloader/bootloader.o $(OBJ)
	$(ARM)-gcc $(C_FLAG) $(LD_FLAG) -T$(BOOTLINKER) $^ -o $@

relo_bootloader.elf: $(SRC_DIR)/main/bootloader/relo_bootloader.o $(OBJ)
	$(ARM)-gcc $(C_FLAG) $(LD_FLAG) -T$(BOOTLINKER) $^ -o $@


sshell: sshell.elf
	$(ARM)-objcopy -O binary $< kernel8.img
	$(ARM)-objdump -D $@.elf > $@.log

# testmbox: testmbox.elf
# 	$(ARM)-objcopy -O binary $< $@.img
# 	$(ARM)-objdump -D $@.elf > $@.log

bootloader: bootloader.elf
	$(ARM)-objcopy -O binary $< $@.img
	$(ARM)-objdump -D $@.elf > $@.log

relo_bootloader: relo_bootloader.elf
	$(ARM)-objcopy -O binary $< $@.img
	$(ARM)-objdump -D $@.elf > $@.log

echo: echo.elf
	$(ARM)-objcopy -O binary $< $@.img
	$(ARM)-objdump -D $@.elf > $@.log

com :
	$(GCC) -g -Wall $(SRC_DIR)/main/utils/raspi3bcom.c -o raspi3bcom

clean:
	rm *.log
	rm *.img
	rm *.elf