SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd -mgeneral-regs-only
CC = clang --target=aarch64-elf
AR = llvm-ar

lib.a: $(OBJS)
	$(AR) r $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f *.o *.a
