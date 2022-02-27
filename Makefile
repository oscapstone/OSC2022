ARM := aarch64-none-linux-gnu


LDFILE := linker.ld
C_FLAG := -g -Wall -static -nostdlib -nostartfiles -fno-builtin-memset
INC := -I./include
LD_FLAG := -static -nostdlib -nostartfiles -T$(LDFILE)

SRC_C := $(wildcard ./src/*.c)
OBJ_C := $(patsubst %.c, %.o, $(SRC_C))
SRC_S := $(wildcard *.S)
OBJ_S := $(patsubst %.S, %.o, $(SRC_S))
SRC := $(SRC_C) $(SRC_S)
OBJ := $(OBJ_C) $(OBJ_S)

%.o: %.c
	$(ARM)-gcc $(INC) $(C_FLAG) -c $< -o $@

%.o: %.S
	$(ARM)-gcc $(C_FLAG) -c $< -o $@

.INTERMEDIATE : $(OBJ)

%.elf: ./src/main/%.o $(OBJ)
	$(ARM)-gcc $(LD_FLAG) $^ -o $@


kernel8: sshell.elf
	$(ARM)-objcopy -O binary $< $@.img
	$(ARM)-objdump -D $< > $@.log

clean:
	rm kernel8.img
	rm kernel8.log
	rm sshell.elf

