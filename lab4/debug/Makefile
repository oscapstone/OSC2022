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
LIBS = \
	 debug.o

all: $(LIBS) 

%.o: %.c
	$(CC) -c $(CFLAGS) -o $(OUTPUT)/$@ $^ -I$(HEADERS)



clean:
	rm $(OUTPUT)/$(LIBS)
