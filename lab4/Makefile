#
# Copyright (C) 2018 bzt (bztsrc@github)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
#
SRC_DIR = src
OUT_DIR = out

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)
OBJS += $(OUT_DIR)/start.o
OBJS += $(OUT_DIR)/main.o
CFLAGS = -Wall -O0 -ffreestanding -nostdinc -nostdlib -nostartfiles -I include

all: clean kernel8.img

$(OUT_DIR)/start.o: start.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

$(OUT_DIR)/main.o: main.c
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	aarch64-linux-gnu-gcc $(CFLAGS) -c $< -o $@

kernel8.img: $(OUT_DIR) $(OBJS)
	aarch64-linux-gnu-ld -nostdlib -nostartfiles $(OBJS) -T link.ld -o kernel8.elf
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
