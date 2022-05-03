SRC_DIR = src
OUT_DIR = out

SRCS = $(wildcard $(SRC_DIR)/*.c)
S_SRCS = $(wildcard $(SRC_DIR)/*.S)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)
S_OBJS = $(S_SRCS:$(SRC_DIR)/%.S=$(OUT_DIR)/%.asmo)
OBJS += $(OUT_DIR)/main.o
CFLAGS = -Wall -O0 -ffreestanding -nostdlib -nostartfiles -I include -mgeneral-regs-only

all: clean kernel8.img

$(OUT_DIR)/main.o: main.c
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

$(OUT_DIR)/%.asmo: $(SRC_DIR)/%.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

kernel8.img: $(OUT_DIR) $(OBJS) $(S_OBJS)
	aarch64-linux-gnu-ld -nostdlib -nostartfiles $(OBJS) $(S_OBJS) -T link.ld -o kernel8.elf
	aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	rm kernel8.elf >/dev/null 2>/dev/null || true
	rm -rf $(OUT_DIR) >/dev/null 2>/dev/null || true

run: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio --display none

gdb: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio --display none -S -s

cpio: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio