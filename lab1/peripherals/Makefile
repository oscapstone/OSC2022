CC = aarch64-linux-gnu-gcc
CPP = aarch64-linux-gnu-cpp
CFLAGS =
CXXFLAGS =

OUTPUT ?= .
HEADERS ?= ../include
OBJECTS = \
		mini_uart.o
		  
.PHONY: all 
all: $(OBJECTS)

%.o : %.c 
	$(CC) -c $^ -o $(OUTPUT)/$@ -I $(HEADERS)

clean:
	rm $(OUTPUT)/$(OBJECTS)
