CC = aarch64-linux-gnu-gcc
CPP = aarch64-linux-gnu-cpp
override CFLAGS += \
        -nostdlib \
        -nodefaultlibs \
        -ffreestanding \
        -nostartfiles \
        -mgeneral-regs-only \
        -fno-builtin \

LDFLAGS ?= \
        -nostdlib \

CXXFLAGS =
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
OBJDUMP = aarch64-linux-gnu-objdump
QEMU = qemu-system-aarch64
RASPI_VER = raspi3b


LINKER_FILE = ./linker.ld
HEADERS = $(PWD)/include/
OUTPUT = $(PWD)/build
KNAME = kernel8
BNAME = bootloader

LIBS = \
	 $(OUTPUT)/print.o \
	 $(OUTPUT)/string.o \
	 $(OUTPUT)/list.o \
	 $(OUTPUT)/cpio.o \
	 $(OUTPUT)/simple_malloc.o \
	 $(OUTPUT)/fdt_parse.o \

KOBJECTS = \
		 $(OUTPUT)/start.o \
		 $(OUTPUT)/mini_uart.o \
		 $(OUTPUT)/mailbox.o \
		 $(OUTPUT)/main.o \
		 $(OUTPUT)/kernel_init.o \
		 $(OUTPUT)/reboot.o \
		 $(OUTPUT)/shell.o \
		 $(OUTPUT)/debug.o \
		 $(OUTPUT)/initrdfs.o \
		 $(LIBS)

BOBJECTS = \
		 $(OUTPUT)/boot.o \
		 $(OUTPUT)/uart_boot.o \
		 $(OUTPUT)/mini_uart.o \
		 $(OUTPUT)/debug.o \
		 $(LIBS)


.PHONY: all start bootloader kernel end clean dump


all: start link bootloader kernel end

start: 
	@echo "Start building kernel8.img"

	$(MAKE) -C boot OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C peripherals OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C init OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)"
	$(MAKE) -C kernel OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C lib OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C debug OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C fs OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 

link: $(OBJECTS)
	$(LD) -T kernel/$(LINKER_FILE) -o $(OUTPUT)/$(KNAME).elf $(KOBJECTS) $(LDFLAGS)
	$(LD) -T boot/$(LINKER_FILE) -o $(OUTPUT)/$(BNAME).elf $(BOBJECTS) $(LDFLAGS) 

bootloader:
	$(OBJCOPY) -O binary $(OUTPUT)/$(BNAME).elf $(OUTPUT)/$(BNAME).img 

kernel: $(OUTPUT)/$(KNAME).elf
	$(OBJCOPY) -O binary $(OUTPUT)/$(KNAME).elf $(OUTPUT)/$(KNAME).img

end: 
	@echo "Finish building kernel8.img"

dump: 
	$(OBJDUMP) -m aarch64 -b binary -D $(OUTPUT)/$(KNAME).img

qemu-run: 
	sudo $(QEMU) -M $(RASPI_VER) -kernel $(OUTPUT)/$(KNAME).img -display none -serial null -serial stdio  -initrd tools/cpio/initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

qemu-run-instr: 
	sudo $(QEMU) -M $(RASPI_VER) -kernel $(OUTPUT)/$(KNAME).img -display none -d in_asm 

clean: 
	rm $(OUTPUT)/*
