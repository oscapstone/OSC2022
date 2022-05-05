CC = aarch64-linux-gnu-gcc
CPP = aarch64-linux-gnu-cpp
CFLAGS =
CXXFLAGS =

OUTPUT ?= ./
OBJECTS = \
	   $(OUTPUT)/boot.o

all: $(OBJECTS) 

$(OBJECTS) : boot.S
	$(CC) -c $^ -o $@


