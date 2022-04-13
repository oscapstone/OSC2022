SRCS = $(wildcard *.c)
CC = clang
CFLAGS = -Wall -g

allocate: $(SRCS)
	$(CC) $^ -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -rf *.o allocate
