CC = aarch64-linux-gnu-gcc
CPP = aarch64-linux-gnu-cpp
CFLAGS ?= \
		-nostdlib \
		-nodefaultlibs \
		-ffreestanding \
		-nostartfiles \
		-mgeneral-regs-only \
		-fno-builtin \
		-g
CXXFLAGS =

OUTPUT ?= .
HEADERS ?= ../include
OBJECTS = \
		shell.o \
		reboot.o \
	    main.o \
		start.o \
		entry.o \
		syscall_test.o \
		syscall_table.o \
		timer.o \
		irq_handler.o \
		signal.o \

.PHONY: all sched
all: $(OBJECTS) sched

sched:
	$(MAKE) -C sched OUTPUT=$(OUTPUT) HEADERS=$(HEADERS) CFLAGS="$(CFLAGS)"

%.o : %.c 
	$(CC) -c $^ -o $(OUTPUT)/$@ -I$(HEADERS) $(CFLAGS)

%.o : %.S
	$(CC) -c $^ -o $(OUTPUT)/$@ -I$(HEADERS) $(CFLAGS)


clean:
	rm $(OUTPUT)/$(OBJECTS)
