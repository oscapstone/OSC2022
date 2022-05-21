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
HEADERS ?= ../include
OBJECTS = \
		boot.o \
		uart_boot.o

all: $(OBJECTS) 

%.o : %.c  
	$(CC) -c $^ -o $(OUTPUT)/$@ -I$(HEADERS) $(CFLAGS) 
 
%.o : %.S 
	$(CC) -c $^ -o $(OUTPUT)/$@ -I$(HEADERS) $(CFLAGS)

clean:
	rm $(OUTPUT)/$(OBJECTS)
