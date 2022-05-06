CC = aarch64-linux-gnu-gcc
CPP = aarch64-linux-gnu-cpp
CFLAGS = 
CXXFLAGS =
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
OBJDUMP = aarch64-linux-gnu-objdump
QEMU = qemu-system-aarch64

LINKER_FILE = ./linker.ld
HEADERS = $(PWD)/include/
OUTPUT = $(PWD)/build
KNAME = $(OUTPUT)/kernel8

OBJECTS = \
		$(OUTPUT)/boot.o \
		$(OUTPUT)/mini_uart.o \
		$(OUTPUT)/main.o \
		$(OUTPUT)/kernel_init.o

.PHONY: all start objects link kernel end clean dump


all: start objects link kernel end

start: 
	@echo "Start building kernel8.img"

objects: 
	$(MAKE) -C boot OUTPUT=$(OUTPUT) HEADERS=$(HEADERS)
	$(MAKE) -C peripherals OUTPUT=$(OUTPUT) HEADERS=$(HEADERS)
	$(MAKE) -C init OUTPUT=$(OUTPUT) HEADERS=$(HEADERS)
	$(MAKE) -C kernel OUTPUT=$(OUTPUT) HEADERS=$(HEADERS)

link: $(OBJECTS)
	$(LD) -T $(LINKER_FILE) -o $(KNAME).elf $(OBJECTS) 

kernel: $(KNAME).elf
	$(OBJCOPY) -O binary $(KNAME).elf $(KNAME).img

end: 
	@echo "Finish building kernel8.img"

dump: 
	$(OBJDUMP) -m aarch64 -b binary -D $(KNAME).img

qemu-run: 
	sudo $(QEMU) -M raspi3 -kernel $(KNAME).img -display none -serial null -serial stdio

qemu-run-instr: 
	sudo $(QEMU) -M raspi3 -kernel $(KNAME).img -display none -d in_asm 

clean: 
	rm $(OUTPUT)/*
