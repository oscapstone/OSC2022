CC = aarch64-linux-gnu-gcc
CPP = aarch64-linux-gnu-cpp
CFLAGS ?= \
        -nostdlib \
        -nodefaultlibs \
        -ffreestanding \
        -nostartfiles \
        -mgeneral-regs-only \
        -fno-builtin

CXXFLAGS =

OUTPUT ?= .
OBJECTS = \
		boot.o \
		uart_boot.o

all: $(OBJECTS) 

%.o: %.S
	$(CC) -c $^ -o $(OUTPUT)/$@

%.o: %.c
	$(CC) -c $^ -o $(OUTPUT)/$@

clean:
	rm $(OUTPUT)/$(OBJECTS)
