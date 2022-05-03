BUILD_DIR = .
# Lib
LIB_SRC_DIR = .

LIB_SRC = \
	# $(LIB_SRC_DIR)/shell.c \
	
LIB_INCLUDES = \
	-I $(LIB_SRC_DIR) \

LIB_BUILD_DIR = $(BUILD_DIR)
LIB_OBJS := $(patsubst %.c,$(LIB_BUILD_DIR)/%.o,$(notdir $(LIB_SRC)))

# Application
APP_SRC_DIR = .

APP_SRC = \
	$(APP_SRC_DIR)/fork_test.c \

APP_ASM = \
	$(APP_SRC_DIR)/start.S \

APP_INCLUDES = \
	-I ./ \
	$(LIB_INCLUDES) \

APP_BUILD_DIR = $(BUILD_DIR)
APP_OBJS := $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_SRC)))
APP_OBJS += $(patsubst %.S,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_ASM)))

VPATH += \
	$(LIB_SRC_DIR) \
	$(APP_SRC_DIR) \

# ARM toolchain
CROSS = aarch64-linux-gnu
CC = $(CROSS)-gcc
AS = $(CROSS)-as
LD = $(CROSS)-ld
OC = $(CROSS)-objcopy
OD = $(CROSS)-objdump
SP = $(CROSS)-strip

# Project
OUT_OBJS = \
	$(LIB_OBJS) \
	$(APP_OBJS) \

BUILD_DIRECTORIES = \
	$(LIB_BUILD_DIR) \
	$(APP_BUILD_DIR) \

PROJ = usr
OUT_ELF = ./$(PROJ).elf
OUT_IMG = ./$(PROJ).img
LINKER_FILE = linker.ld

# CCFLAGS = -Wall -O0 -ffreestanding -nostdinc -nostdlib -nostartfiles
CCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -g
LDFLAGS = -T$(LINKER_FILE)

.PHONY:  all clean
all: cpio

# Compile Object Files ---------------------------------------------------------
$(APP_BUILD_DIR)/%.o : %.c
	@echo "[APP Objects] : $@ -----------------------------------------"
	@echo "Building: $<"
	$(CC) $(CCFLAGS) $(APP_INCLUDES) -o $@ -c $<
	@echo "Finished Building: $<"

$(APP_BUILD_DIR)/%.o : %.S
	@echo "[APP Objects] : $@ -----------------------------------------"
	@echo "Building: $<"
	$(CC) $(ASMFLAGS) $(APP_INCLUDES) -o $@ -c $<
	@echo "Finished Building: $<"

$(LIB_BUILD_DIR)/%.o : %.c
	@echo "[LIB Objects] : $@ -----------------------------------------"
	@echo "Building: $<"
	$(CC) $(CCFLAGS) $(LIB_INCLUDES) -o $@ -c $<
	@echo "Finished Building: $<"

# Generate ELF -----------------------------------------------------------------
$(OUT_ELF): $(OUT_OBJS)
	@echo '============================================================'
	@echo 'Building target: $@'
	@echo '--------------------------------'
	$(LD) $(LDFLAGS) -o $@ $(OUT_OBJS)
	$(OD) -d $@ > $(BUILD_DIR)/$(PROJ).objdump
	@echo 'Finished building target: $@'
	@echo ' '

# Generate IMG -----------------------------------------------------------------
$(OUT_IMG): $(OUT_ELF)
	@echo '============================================================'
	@echo 'Building target: $@'
	@echo '--------------------------------'
	$(OC) -O binary $(OUT_ELF) $@
	@echo 'Finished building target: $@'
	@echo ' '

$(BUILD_DIRECTORIES):
	mkdir -p $@

cpio: $(OUT_IMG)
	cp $(OUT_IMG) ./rootfs/$(OUT_IMG)
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.img
	cd ./rootfs; find . | cpio -o -H newc > ../initramfs.cpio

clean:
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.img
	rm -rf *.cpio