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
		 page_alloc.o \
		 slab.o \
		 mmu.o \
		 mm.o

.PHONY: all 
all: $(OBJECTS)

%.o : %.c 
	$(CC) -c $^ -o $(OUTPUT)/$@ -I $(HEADERS) $(CFLAGS)

clean:
	rm $(OUTPUT)/$(OBJECTS)
