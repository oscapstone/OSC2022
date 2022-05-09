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
KNAME = $(OUTPUT)/kernel8

LIBS = \
	 $(OUTPUT)/print.o \
	 $(OUTPUT)/string.o \

OBJECTS = \
		$(OUTPUT)/boot.o \
		$(OUTPUT)/mini_uart.o \
		$(OUTPUT)/main.o \
		$(OUTPUT)/kernel_init.o \
		$(OUTPUT)/shell.o \
		$(OUTPUT)/debug.o \
		$(LIBS)


.PHONY: all start objects link kernel end clean dump


all: start objects link kernel end

start: 
	@echo "Start building kernel8.img"

objects: 
	$(MAKE) -C boot OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C peripherals OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C init OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)"
	$(MAKE) -C kernel OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C lib OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 
	$(MAKE) -C debug OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)" 

link: $(OBJECTS)
	$(LD) -T $(LINKER_FILE) -o $(KNAME).elf $(OBJECTS) $(LDFLAGS)

kernel: $(KNAME).elf
	$(OBJCOPY) -O binary $(KNAME).elf $(KNAME).img

end: 
	@echo "Finish building kernel8.img"

dump: 
	$(OBJDUMP) -m aarch64 -b binary -D $(KNAME).img

qemu-run: 
	sudo $(QEMU) -M $(RASPI_VER) -kernel $(KNAME).img -display none -serial null -serial stdio

qemu-run-instr: 
	sudo $(QEMU) -M $(RASPI_VER) -kernel $(KNAME).img -display none -d in_asm 

clean: 
	rm $(OUTPUT)/*
